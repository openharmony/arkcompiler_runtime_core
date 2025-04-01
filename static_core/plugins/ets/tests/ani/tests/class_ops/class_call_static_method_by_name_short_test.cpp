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

class ClassCallStaticMethodByNameShortTest : public AniTest {
public:
    static constexpr ani_short VAL1 = 5;
    static constexpr ani_short VAL2 = 6;
    void GetClassData(ani_class *clsResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_short_test/Operations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
    void TestFuncV(ani_class cls, const char *name, ani_short *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_V(cls, name, "SS:S", value, args), ANI_OK);
        va_end(args);
    }
};

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, "sum", nullptr, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "sum", nullptr, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "sum", nullptr, &value, args), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_null_class)
{
    ani_short value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, nullptr, "sum", nullptr, &value, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_null_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, nullptr, nullptr, &value, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_error_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, "aa", nullptr, &value, VAL1, VAL2),
              ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_null_result)
{
    ani_class cls {};
    GetClassData(&cls);

    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, "sum", nullptr, nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v_null_class)
{
    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(nullptr, "sum", nullptr, &value, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v_null_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, nullptr, nullptr, &value, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v_error_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "aa", nullptr, &value, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v_null_result)
{
    ani_class cls {};
    GetClassData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "sum", nullptr, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_null_class)
{
    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(nullptr, "sum", nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_null_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, nullptr, nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_error_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "aa", nullptr, &value, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_null_result)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "sum", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_null_args)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "sum", nullptr, &value, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_short_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "funcA", "SS:S", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "funcA", "SS:S", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_short valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_2)
{
    ani_namespace nb {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_short_test/nb;", &nb), ANI_OK);
    ani_namespace nc {};
    ASSERT_EQ(env_->Namespace_FindNamespace(nb, "Lnc;", &nc), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(nc, "LA;", &cls), ANI_OK);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "funcA", "SS:S", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "funcA", "SS:S", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_short valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_3)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_short_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "funcA", "SS:S", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "funcA", "SS:S", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_short valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);

    ani_int valueI;
    const ani_int value1 = 4;
    const ani_int value2 = 7;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "II:I", &valueI, value1, value2), ANI_OK);
    ASSERT_EQ(valueI, value2 - value1);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_4)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_short_test/nd;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LB;", &cls), ANI_OK);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "funcA", "SS:S", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "funcA", "SS:S", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);

    ani_short valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL2 - VAL1);
}
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
