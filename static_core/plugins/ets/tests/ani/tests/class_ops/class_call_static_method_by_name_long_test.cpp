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

class ClassCallStaticMethodByNameLongTest : public AniTest {
public:
    static constexpr ani_long VAL1 = 123;
    static constexpr ani_long VAL2 = 456;

    void GetMethodData(ani_class *clsResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_long_test/Operations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
    void TestFuncV(ani_class cls, const char *name, ani_long *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Long_V(cls, name, "JJ:J", value, args), ANI_OK);
        va_end(args);
    }
};

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_long sum = 0L;
    const ani_long v1 = VAL1;
    const ani_long v2 = VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Long(env_, cls, "sum", nullptr, &sum, v1, v2), ANI_OK);
    ASSERT_EQ(sum, v1 + v2);
}

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long_v)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_long sum = 0L;
    const ani_long v1 = VAL1;
    const ani_long v2 = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long(cls, "sum", nullptr, &sum, v1, v2), ANI_OK);
    ASSERT_EQ(sum, v1 + v2);
}

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long_A)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long sum = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long_A(cls, "sum", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long_null_class)
{
    ani_long sum = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long(nullptr, "sum", nullptr, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long_null_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_long sum = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long(cls, nullptr, nullptr, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long(cls, "sum_not_exist", nullptr, &sum, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long_null_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long(cls, "sum", nullptr, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long_A_null_class)
{
    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long sum = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long_A(nullptr, "sum", nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long_A_null_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long sum = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long_A(cls, nullptr, nullptr, &sum, args), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long_A(cls, "sum_not_exist", nullptr, &sum, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long_A_null_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long_A(cls, "sum", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long_A_null_args)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_long sum = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long_A(cls, "sum", nullptr, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long_combine_scenes_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_long_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ani_long value = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long(cls, "funcA", "JJ:J", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long valueA = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long_A(cls, "funcA", "JJ:J", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_long valueV = 0L;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long_combine_scenes_2)
{
    ani_namespace nb {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_long_test/nb;", &nb), ANI_OK);
    ani_namespace nc {};
    ASSERT_EQ(env_->Namespace_FindNamespace(nb, "Lnc;", &nc), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(nc, "LA;", &cls), ANI_OK);

    ani_long value = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long(cls, "funcA", "JJ:J", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long valueA = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long_A(cls, "funcA", "JJ:J", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_long valueV = 0L;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long_combine_scenes_3)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_long_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ani_long value = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long(cls, "funcA", "JJ:J", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long valueA = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long_A(cls, "funcA", "JJ:J", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_long valueV = 0L;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);

    ani_int value2 = 0;
    const ani_int value3 = 4;
    const ani_int value4 = 7;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "II:I", &value2, value3, value4), ANI_OK);
    ASSERT_EQ(value2, value4 - value3);
}

TEST_F(ClassCallStaticMethodByNameLongTest, call_static_method_by_name_long_combine_scenes_4)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_long_test/nd;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LB;", &cls), ANI_OK);

    ani_long value = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long(cls, "funcA", "JJ:J", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long valueA = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Long_A(cls, "funcA", "JJ:J", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);

    ani_long valueV = 0L;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL2 - VAL1);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
