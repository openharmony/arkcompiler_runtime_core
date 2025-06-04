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
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
