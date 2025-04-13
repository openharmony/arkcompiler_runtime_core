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

class ClassCallStaticMethodByNameFloatTest : public AniTest {
public:
    static constexpr ani_float FLOAT_VAL1 = 1.5F;
    static constexpr ani_float FLOAT_VAL2 = 2.5F;
    static constexpr ani_int VAL3 = 5;
    static constexpr ani_int VAL4 = 6;
    static constexpr size_t ARG_COUNT = 2U;

    void GetMethodData(ani_class *clsResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_float_test/Operations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
    void TestFuncV(ani_class cls, const char *name, ani_float *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_V(cls, name, "FF:F", value, args), ANI_OK);
        va_end(args);
    }

    void TestCombineScene(const char *className, const char *methodName, ani_float expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ani_float value = 0.0;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, methodName, "FF:F", &value, FLOAT_VAL1, FLOAT_VAL2),
                  ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].f = FLOAT_VAL1;
        args[1U].f = FLOAT_VAL2;
        ani_float valueA = 0.0;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, methodName, "FF:F", &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_float sum {};
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Float(env_, cls, "sum", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_OK);
    ASSERT_EQ(sum, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_v)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "sum", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(sum, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "sum", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_null_class)
{
    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(nullptr, "sum", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_null_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, nullptr, nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "sum_not_exist", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "\n", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_null_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "sum", nullptr, nullptr, FLOAT_VAL1, FLOAT_VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A_null_class)
{
    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(nullptr, "sum", nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A_null_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, nullptr, nullptr, &sum, args), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "sum_not_exist", nullptr, &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "", nullptr, &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "\n", nullptr, &sum, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A_null_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "sum", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A_null_args)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "sum", nullptr, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_float_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "funcA", "FF:F", &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "funcA", "FF:F", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);

    ani_float valueV = 0.0F;
    TestFuncV(cls, "funcA", &valueV, FLOAT_VAL1, FLOAT_VAL2);
    ASSERT_EQ(valueV, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_2)
{
    ani_namespace nb {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_float_test/nb;", &nb), ANI_OK);
    ani_namespace nc {};
    ASSERT_EQ(env_->Namespace_FindNamespace(nb, "Lnc;", &nc), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(nc, "LA;", &cls), ANI_OK);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "funcA", "FF:F", &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "funcA", "FF:F", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);

    ani_float valueV = 0.0F;
    TestFuncV(cls, "funcA", &valueV, FLOAT_VAL1, FLOAT_VAL2);
    ASSERT_EQ(valueV, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_3)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_float_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "funcA", "FF:F", &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "funcA", "FF:F", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);

    ani_float valueV = 0.0F;
    TestFuncV(cls, "funcA", &valueV, FLOAT_VAL1, FLOAT_VAL2);
    ASSERT_EQ(valueV, FLOAT_VAL1 + FLOAT_VAL2);

    ani_int value2 = 0;
    const ani_int value3 = 4;
    const ani_int value4 = 7;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "II:I", &value2, value3, value4), ANI_OK);
    ASSERT_EQ(value2, value4 - value3);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_4)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_float_test/nd;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LB;", &cls), ANI_OK);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "funcA", "FF:F", &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL2 - FLOAT_VAL1);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "funcA", "FF:F", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL2 - FLOAT_VAL1);

    ani_float valueV = 0.0F;
    TestFuncV(cls, "funcA", &valueV, FLOAT_VAL1, FLOAT_VAL2);
    ASSERT_EQ(valueV, FLOAT_VAL2 - FLOAT_VAL1);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_null_env)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_float value = 0.0;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethodByName_Float(nullptr, cls, "or", nullptr, &value, FLOAT_VAL1, FLOAT_VAL2),
        ANI_INVALID_ARGS);
    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Float_A(nullptr, cls, "or", nullptr, &value, args),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_5)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_float_test/A;", &clsA), ANI_OK);
    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_float_test/B;", &clsB), ANI_OK);

    ani_float valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(clsA, "funcA", "FF:F", &valueA, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);

    ani_float valueB = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(clsB, "funcB", "FF:F", &valueB, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(valueB, FLOAT_VAL2 - FLOAT_VAL1);

    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueAA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(clsA, "funcA", "FF:F", &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, FLOAT_VAL1 + FLOAT_VAL2);
    ani_float valueBA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(clsB, "funcB", "FF:F", &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, FLOAT_VAL2 - FLOAT_VAL1);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_6)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_float_test/A;", &cls), ANI_OK);
    ani_float value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "funcA", "FF:F", &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "funcA", "FF:F", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);

    ani_int value2 = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "II:I", &value2, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(value2, VAL3 + VAL4);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_7)
{
    TestCombineScene("Lclass_call_static_method_by_name_float_test/A;", "funcB", FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_8)
{
    TestCombineScene("Lclass_call_static_method_by_name_float_test/C;", "funcA", FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_9)
{
    TestCombineScene("Lclass_call_static_method_by_name_float_test/D;", "funcA", FLOAT_VAL2 - FLOAT_VAL1);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_10)
{
    TestCombineScene("Lclass_call_static_method_by_name_float_test/E;", "funcA", FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_11)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_float_test/F;", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "increment", nullptr, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ani_float value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "getCount", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "getCount", nullptr, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_12)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_float_test/G;", &cls), ANI_OK);
    ani_float value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "publicMethod", "FF:F", &value, FLOAT_VAL1, FLOAT_VAL2),
              ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);
    ASSERT_EQ(
        env_->Class_CallStaticMethodByName_Float(cls, "callPrivateMethod", "FF:F", &value, FLOAT_VAL1, FLOAT_VAL2),
        ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL2 - FLOAT_VAL1);

    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "publicMethod", "FF:F", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "callPrivateMethod", "FF:F", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL2 - FLOAT_VAL1);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
