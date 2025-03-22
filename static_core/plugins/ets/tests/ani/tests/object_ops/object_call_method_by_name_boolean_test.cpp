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

class CallObjectMethodBooleanByNameTest : public AniTest {
public:
    static constexpr ani_int VAL1 = 5;
    static constexpr ani_int VAL2 = 6;

    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_boolean_test/A;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod {};
        ASSERT_EQ(
            env_->Class_FindStaticMethod(cls, "new_A", ":Lobject_call_method_by_name_boolean_test/A;", &newMethod),
            ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);
        *objectResult = static_cast<ani_object>(ref);
    }
};

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_boolean_a)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(object, "booleanByNameMethod", "II:Z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_boolean_v)
{
    ani_object object {};
    GetMethodData(&object);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "booleanByNameMethod", "II:Z", &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_boolean_v_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "booleanByNameMethod", "II:X", &res, VAL1, VAL2),
              ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "unknown_function", "II:Z", &res, VAL1, VAL2),
              ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_boolean)
{
    ani_object object {};
    GetMethodData(&object);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(
        env_->c_api->Object_CallMethodByName_Boolean(env_, object, "booleanByNameMethod", "II:Z", &res, VAL1, VAL2),
        ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, call_method_boolean_v_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, nullptr, "II:Z", &res, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanByNameTest, call_method_boolean_v_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "booleanByNameMethod", "II:Z", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanByNameTest, call_method_boolean_v_invalid_object)
{
    ani_object object {};
    GetMethodData(&object);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(nullptr, "booleanByNameMethod", "II:Z", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanByNameTest, call_method_boolean_a_invalid_args)
{
    ani_object object {};
    GetMethodData(&object);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(nullptr, "booleanByNameMethod", "II:Z", &res, nullptr),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_boolean_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "II:Z", &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "booleanMethod", "II:Z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_boolean_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    const ani_int value = 5;
    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "I:Z", &res, value), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ani_value args[1U];
    args[0U].i = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "booleanMethod", "I:Z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_boolean_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanCompareMethod", "II:Z", &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "booleanCompareMethod", "II:Z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_boolean_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "II:Z", &res, VAL1, VAL1), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "booleanMethod", "II:Z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_boolean_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 5;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ani_value args[1U];
    args[0U].i = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "protectedMethod", "I:Z", &res, VAL1), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "protectedMethod", "I:Z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "privateMethod", "I:Z", &res, VAL1), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "privateMethod", "I:Z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_boolean_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &method), ANI_OK);
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "callProtected", "I:Z", &res, VAL1), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "callProtected", "I:Z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_boolean_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL1), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "nestedMethod", "I:Z", &res, VAL2), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    ani_value args[1U];
    args[0U].i = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "nestedMethod", "I:Z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_boolean_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ani_value argsA[1];
    argsA[0].i = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "recursiveMethod", "I:Z", &res, VAL1), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "recursiveMethod", "I:Z", &res, argsA), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    const ani_int value1 = -5;
    ani_value argsB[1];
    argsB[0].i = value1;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "recursiveMethod", "I:Z", &res, value1), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "recursiveMethod", "I:Z", &res, argsB), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_008)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_boolean_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ani_char cValue1 = 'A';
    ani_double dValue1 = 1.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "calculateSum", nullptr, &res, VAL1, cValue1, dValue1),
              ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ani_char cValue2 = 'B';
    ani_value args[3U];
    args[0U].i = VAL1;
    args[1U].c = cValue2;
    args[2U].d = dValue1;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "calculateSum", nullptr, &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    ani_double dValue2 = 2U;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "calculateSum", nullptr, &res, VAL1, cValue2, dValue2),
              ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_009)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_boolean_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 15;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_boolean res = ANI_FALSE;
    const ani_int value1 = 5;
    const ani_int value2 = 6;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "II:Z", &res, value1, value2), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    const ani_int value3 = 7;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "II:Z", &res, value1, value3), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    const ani_int value4 = 3;
    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value4;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "booleanMethod", "II:Z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    const ani_int value5 = 5;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "II:Z", &res, value1, value5), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    const ani_int value6 = 12;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "II:Z", &res, value1, value6), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_010)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_boolean_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 10;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_boolean res = ANI_FALSE;
    const ani_int value = 10;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "jf", "I:Z", &res, value), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ani_value args[1U];
    args[0U].i = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "jf", "I:Z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)