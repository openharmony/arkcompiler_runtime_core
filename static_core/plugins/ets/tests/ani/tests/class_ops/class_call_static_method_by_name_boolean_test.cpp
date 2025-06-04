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

class ClassCallStaticMethodByNameBooleanTest : public AniTest {
public:
    static constexpr ani_int VAL1 = 5U;
    static constexpr ani_int VAL2 = 6U;
    void GetClassData(ani_class *clsResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_boolean_test/Operations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
    void TestFuncV(ani_class cls, const char *name, ani_boolean *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_V(cls, name, "ZZ:Z", value, args), ANI_OK);
        va_end(args);
    }

    void TestCombineScene(const char *className, const char *methodName, ani_boolean initValue,
                          ani_boolean expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ani_boolean value = initValue;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, methodName, "ZZ:Z", &value, ANI_TRUE, ANI_FALSE),
                  ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].z = ANI_TRUE;
        args[1U].z = ANI_FALSE;
        ani_boolean valueA = initValue;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, methodName, "ZZ:Z", &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Boolean(env_, cls, "or", nullptr, &value, ANI_TRUE, ANI_FALSE),
              ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_v)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "or", nullptr, &value, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_A)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "or", nullptr, &value, args), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_null_class)
{
    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethodByName_Boolean(env_, nullptr, "or", nullptr, &value, ANI_TRUE, ANI_FALSE),
        ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_null_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethodByName_Boolean(env_, cls, nullptr, nullptr, &value, ANI_TRUE, ANI_FALSE),
        ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_error_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Boolean(env_, cls, "aa", nullptr, &value, ANI_TRUE, ANI_FALSE),
              ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "", nullptr, &value, ANI_TRUE, ANI_FALSE), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "\n", nullptr, &value, ANI_TRUE, ANI_FALSE),
              ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_null_result)
{
    ani_class cls {};
    GetClassData(&cls);

    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Boolean(env_, cls, "or", nullptr, nullptr, ANI_TRUE, ANI_FALSE),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_v_null_class)
{
    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(nullptr, "or", nullptr, &value, ANI_TRUE, ANI_FALSE),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_v_null_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, nullptr, nullptr, &value, ANI_TRUE, ANI_FALSE),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_v_error_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "aa", nullptr, &value, ANI_TRUE, ANI_FALSE),
              ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_v_null_result)
{
    ani_class cls {};
    GetClassData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "or", nullptr, nullptr, ANI_TRUE, ANI_FALSE),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_A_null_class)
{
    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(nullptr, "or", nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_A_null_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, nullptr, nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_A_error_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "aa", nullptr, &value, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "", nullptr, &value, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "\n", nullptr, &value, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_A_null_result)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "or", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_A_null_args)
{
    ani_class cls {};
    GetClassData(&cls);
    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "or", nullptr, &value, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_combine_scenes_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_boolean_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "funcA", "ZZ:Z", &value, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean valueA = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "funcA", "ZZ:Z", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, ANI_TRUE);

    ani_boolean valueV = ANI_FALSE;
    TestFuncV(cls, "funcA", &valueV, ANI_TRUE, ANI_FALSE);
    ASSERT_EQ(valueV, ANI_TRUE);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_combine_scenes_2)
{
    ani_namespace nb {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_boolean_test/nb;", &nb), ANI_OK);
    ani_namespace nc {};
    ASSERT_EQ(env_->Namespace_FindNamespace(nb, "Lnc;", &nc), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(nc, "LA;", &cls), ANI_OK);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "funcA", "ZZ:Z", &value, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean valueA = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "funcA", "ZZ:Z", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, ANI_TRUE);

    ani_boolean valueV = ANI_FALSE;
    TestFuncV(cls, "funcA", &valueV, ANI_TRUE, ANI_FALSE);
    ASSERT_EQ(valueV, ANI_TRUE);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_combine_scenes_3)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_boolean_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "funcA", "ZZ:Z", &value, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean valueA = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "funcA", "ZZ:Z", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, ANI_TRUE);

    ani_boolean valueV = ANI_FALSE;
    TestFuncV(cls, "funcA", &valueV, ANI_TRUE, ANI_FALSE);
    ASSERT_EQ(valueV, ANI_TRUE);

    ani_int valueI = 0;
    const ani_int value1 = 5;
    const ani_int value2 = 6;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "II:I", &valueI, value1, value2), ANI_OK);
    ASSERT_EQ(valueI, value1 + value2);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_combine_scenes_4)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_boolean_test/nd;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LB;", &cls), ANI_OK);
    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "funcA", "ZZ:Z", &value, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(value, ANI_FALSE);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean valueA = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "funcA", "ZZ:Z", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, ANI_FALSE);

    ani_boolean valueV = ANI_FALSE;
    TestFuncV(cls, "funcA", &valueV, ANI_TRUE, ANI_FALSE);
    ASSERT_EQ(valueV, ANI_FALSE);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_null_env)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethodByName_Boolean(nullptr, cls, "or", nullptr, &value, ANI_TRUE, ANI_FALSE),
        ANI_INVALID_ARGS);
    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Boolean_A(nullptr, cls, "or", nullptr, &value, args),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_combine_scenes_5)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_boolean_test/A;", &clsA), ANI_OK);
    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_boolean_test/B;", &clsB), ANI_OK);

    ani_boolean valueA = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(clsA, "funcA", "ZZ:Z", &valueA, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(valueA, ANI_TRUE);
    ani_boolean valueB = ANI_TRUE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(clsB, "funcB", "ZZ:Z", &valueB, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(valueB, ANI_FALSE);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean valueAA = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(clsA, "funcA", "ZZ:Z", &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, ANI_TRUE);
    ani_boolean valueBA = ANI_TRUE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(clsB, "funcB", "ZZ:Z", &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, ANI_FALSE);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_combine_scenes_6)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_boolean_test/A;", &cls), ANI_OK);
    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "funcA", "ZZ:Z", &value, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean valueA = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "funcA", "ZZ:Z", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, ANI_TRUE);

    ani_int value2 = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "II:I", &value2, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value2, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_combine_scenes_7)
{
    TestCombineScene("Lclass_call_static_method_by_name_boolean_test/A;", "funcB", ANI_FALSE, ANI_TRUE);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_combine_scenes_8)
{
    TestCombineScene("Lclass_call_static_method_by_name_boolean_test/C;", "funcA", ANI_FALSE, ANI_TRUE);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_combine_scenes_9)
{
    TestCombineScene("Lclass_call_static_method_by_name_boolean_test/D;", "funcA", ANI_TRUE, ANI_FALSE);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_combine_scenes_10)
{
    TestCombineScene("Lclass_call_static_method_by_name_boolean_test/E;", "funcA", ANI_FALSE, ANI_TRUE);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_combine_scenes_11)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_boolean_test/F;", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "increment", nullptr, ANI_TRUE, ANI_FALSE), ANI_OK);
    ani_boolean value = ANI_TRUE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "getCount", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, ANI_FALSE);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean valueA = ANI_TRUE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "getCount", nullptr, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, ANI_FALSE);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, call_static_method_by_name_bool_combine_scenes_12)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_boolean_test/G;", &cls), ANI_OK);
    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "publicMethod", "ZZ:Z", &value, ANI_TRUE, ANI_FALSE),
              ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "callPrivateMethod", "ZZ:Z", &value, ANI_TRUE, ANI_FALSE),
              ANI_OK);
    ASSERT_EQ(value, ANI_FALSE);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean valueA = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "publicMethod", "ZZ:Z", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, ANI_TRUE);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "callPrivateMethod", "ZZ:Z", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, ANI_FALSE);
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, check_initialization_boolean)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_boolean_test.G", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_boolean_test.G"));
    ani_boolean value {};

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "publicMethodx", "zz:z", &value, ANI_TRUE, ANI_FALSE),
              ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_boolean_test.G"));

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "publicMethod", "zz:z", &value, ANI_TRUE, ANI_FALSE),
              ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_call_static_method_by_name_boolean_test.G"));
}

TEST_F(ClassCallStaticMethodByNameBooleanTest, check_initialization_boolean_a)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_boolean_test.G", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_boolean_test.G"));
    ani_boolean value {};
    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "publicMethodx", "zz:z", &value, args), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_boolean_test.G"));

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "publicMethod", "zz:z", &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_call_static_method_by_name_boolean_test.G"));
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
