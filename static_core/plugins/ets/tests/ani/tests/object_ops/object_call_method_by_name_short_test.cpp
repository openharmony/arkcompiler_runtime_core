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

class CallObjectMethodShortByNameTest : public AniTest {
public:
    static constexpr ani_short VAL0 = 100;
    static constexpr ani_short VAL1 = 5;
    static constexpr ani_short VAL2 = 6;

    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_short_test/A;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":Lobject_call_method_by_name_short_test/A;", &newMethod),
                  ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        *objectResult = static_cast<ani_object>(ref);
    }
};

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_a)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;

    ani_short res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(object, "shortByNameMethod", "SS:S", &res, args), ANI_OK);
    ASSERT_EQ(res, VAL1 + VAL2);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_v)
{
    ani_object object {};
    GetMethodData(&object);

    ani_short res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(object, "shortByNameMethod", "SS:S", &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(res, VAL1 + VAL2);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short)
{
    ani_object object {};
    GetMethodData(&object);

    ani_short res {};
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Short(env_, object, "shortByNameMethod", "SS:S", &res, VAL1, VAL2),
              ANI_OK);
    ASSERT_EQ(res, VAL1 + VAL2);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_v_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_short res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(object, "shortByNameMethod", "SS:X", &res, VAL1, VAL2),
              ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Short(object, "unknown_function", "SS:S", &res, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_v_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_short res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(object, nullptr, "SS:S", &res, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_v_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Short(object, "shortByNameMethod", "SS:S", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_v_invalid_object)
{
    ani_object object {};
    GetMethodData(&object);

    ani_short res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(nullptr, "shortByNameMethod", "SS:S", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_a_invalid_args)
{
    ani_object object {};
    GetMethodData(&object);

    ani_short res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(nullptr, "shortByNameMethod", "SS:S", &res, nullptr),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_short_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "S:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_short sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "SS:S", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL0);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortMethod", "SS:S", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_short_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "S:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_short sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "S:S", &sum, VAL1), ANI_OK);
    ASSERT_EQ(sum, VAL0);

    ani_value args[1U];
    args[0U].s = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortMethod", "S:S", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_short_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "S:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_short sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortAddMethod", "SS:S", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL0 + VAL1 + VAL2);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortAddMethod", "SS:S", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0 + VAL1 + VAL2);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_short_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "S:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_short sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "SS:S", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL0 - VAL1 - VAL2);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortMethod", "SS:S", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0 - VAL1 - VAL2);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_short_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "S:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_short num {};
    ani_value args[1U];
    args[0U].s = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "protectedMethod", "S:S", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "protectedMethod", "S:S", &num, args), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);

    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "privateMethod", "S:S", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, VAL0 - VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "privateMethod", "S:S", &num, args), ANI_OK);
    ASSERT_EQ(num, VAL0 - VAL1);

    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_short_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "S:V", &method), ANI_OK);
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "callProtected", "S:S", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "callProtected", "S:S", &num, args), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_short_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "S:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_short sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "nestedMethod", "S:S", &sum, VAL1), ANI_OK);
    ASSERT_EQ(sum, VAL2 + VAL1);

    ani_value args[1U];
    args[0U].s = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "nestedMethod", "S:S", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL2 + VAL1);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_short_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "S:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_short sum {};
    const ani_int value1 = 5;
    const ani_short result = 120;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "recursiveMethod", "I:S", &sum, value1), ANI_OK);
    ASSERT_EQ(sum, result);

    ani_value args[1U];
    args[0U].i = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "recursiveMethod", "I:S", &sum, args), ANI_OK);
    ASSERT_EQ(sum, result);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_008)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_short_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "S:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_short sum {};
    const ani_short value1 = 1;
    const ani_char value2 = 'A';
    const ani_double value3 = 1.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "calculateSum", "SCD:S", &sum, value1, value2, value3), ANI_OK);
    ASSERT_EQ(sum, VAL2 - value1);

    const ani_char value4 = 'B';
    ani_value args[3U];
    args[0U].s = value1;
    args[1U].c = value4;
    args[2U].d = value3;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "calculateSum", "SCD:S", &sum, args), ANI_OK);
    ASSERT_EQ(sum, value1);

    const ani_double value5 = 2U;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "calculateSum", "SCD:S", &sum, value1, value4, value5), ANI_OK);
    ASSERT_EQ(sum, VAL2 + value1);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_009)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_short_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "S:V", &method), ANI_OK);

    ani_object obj {};
    const ani_short arg = 15;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_short sum {};
    const ani_short value1 = 5;
    const ani_short value2 = 6;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "SS:S", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_short value3 = 7;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "SS:S", &sum, value1, value3), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_short value4 = 3;
    ani_value args[2U];
    args[0U].s = value1;
    args[1U].s = value4;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortMethod", "SS:S", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_short value5 = 10;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "SS:S", &sum, value1, value5), ANI_OK);
    ASSERT_EQ(sum, value1 + value5);

    const ani_short value6 = 12;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "SS:S", &sum, value1, value6), ANI_OK);
    ASSERT_EQ(sum, value1 + value6);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_010)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_short_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "S:V", &method), ANI_OK);

    ani_object obj {};
    const ani_short arg = 10;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_short sum {};
    const ani_short value = 2;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "jf", "S:S", &sum, value), ANI_OK);
    ASSERT_EQ(sum, arg + value);

    ani_value args[1U];
    args[0U].s = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "jf", "S:S", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg + value);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_011)
{
    ani_object obj {};
    GetMethodData(&obj);

    ani_short sum = 0;
    const ani_short value1 = std::numeric_limits<ani_short>::max();
    const ani_short value2 = 0;
    ani_value args1[2U];
    args1[0U].s = value1;
    args1[1U].s = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortByNameMethod", "SS:S", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortByNameMethod", "SS:S", &sum, args1), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);

    const ani_short value3 = std::numeric_limits<ani_short>::min();
    ani_value args2[2U];
    args2[0U].s = value3;
    args2[1U].s = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortByNameMethod", "SS:S", &sum, value3, value2), ANI_OK);
    ASSERT_EQ(sum, value3 + value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortByNameMethod", "SS:S", &sum, args2), ANI_OK);
    ASSERT_EQ(sum, value3 + value2);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)