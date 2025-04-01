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

class CallObjectMethodCharByNameTest : public AniTest {
public:
    static constexpr ani_char VAL = 'a';
    static constexpr ani_char VAL1 = 'C';
    static constexpr ani_char VAL2 = 'A';
    static constexpr ani_char VAL3 = 'H';

    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_char_test/A;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":Lobject_call_method_by_name_char_test/A;", &newMethod),
                  ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);
        *objectResult = static_cast<ani_object>(ref);
    }
};

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_a)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].c = VAL2;
    args[1U].c = VAL1;

    ani_char res = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(object, "charByNameMethod", "CC:C", &res, args), ANI_OK);
    ASSERT_EQ(res, VAL1);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v)
{
    ani_object object {};
    GetMethodData(&object);

    ani_char res = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "charByNameMethod", "CC:C", &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(res, VAL1);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char)
{
    ani_object object {};
    GetMethodData(&object);

    ani_char res = VAL;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Char(env_, object, "charByNameMethod", "CC:C", &res, VAL1, VAL2),
              ANI_OK);
    ASSERT_EQ(res, VAL1);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_char res = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "charByNameMethod", "CC:X", &res, VAL1, VAL2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "unknown_function", "CC:C", &res, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_char res = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, nullptr, "CC:C", &res, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "charByNameMethod", "CC:C", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v_invalid_object)
{
    ani_object object {};
    GetMethodData(&object);

    ani_char res = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(nullptr, "charByNameMethod", "CC:C", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_a_invalid_args)
{
    ani_object object {};
    GetMethodData(&object);

    ani_char res = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(nullptr, "charByNameMethod", "CC:C", &res, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_char_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "CC:C", &sum, VAL2, VAL1), ANI_OK);
    ASSERT_EQ(sum, VAL1);

    ani_value args[2U];
    args[0U].c = VAL2;
    args[1U].c = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "charMethod", "CC:C", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_char_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    const ani_char value1 = 'A';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "C:C", &sum, value1), ANI_OK);
    ASSERT_EQ(sum, VAL3);

    const ani_char value2 = 'Z';
    ani_value args[1U];
    args[0U].c = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "charMethod", "C:C", &sum, args), ANI_OK);
    ASSERT_EQ(sum, value2);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_char_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charComparisonMethod", "CC:C", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1);

    ani_value args[2U];
    args[0U].c = VAL1;
    args[1U].c = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "charComparisonMethod", "CC:C", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_char_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "CC:C", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL2);

    ani_value args[2U];
    args[0U].c = VAL1;
    args[1U].c = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "charMethod", "CC:C", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL2);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_char_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char num;
    const ani_char value1 = 'A';
    const ani_char value2 = 'V';
    ani_value argsA[1U];
    argsA[0U].c = value1;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "protectedMethod", "C:C", &num, value1), ANI_OK);
    ASSERT_EQ(num, VAL3);
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "protectedMethod", "C:C", &num, argsA), ANI_OK);
    ASSERT_EQ(num, VAL3);

    ani_value argsB[2U];
    argsB[0U].c = value1;
    argsB[1U].c = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "privateMethod", "CC:C", &num, value1, value2), ANI_OK);
    ASSERT_EQ(num, value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "privateMethod", "CC:C", &num, argsB), ANI_OK);
    ASSERT_EQ(num, value2);

    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_char_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C:V", &method), ANI_OK);
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "callProtected", "C:C", &num, value1), ANI_OK);
    ASSERT_EQ(num, VAL3);

    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "callProtected", "C:C", &num, argsA), ANI_OK);
    ASSERT_EQ(num, VAL3);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_char_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    const ani_char value = 'D';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "nestedMethod", "C:C", &sum, value), ANI_OK);
    ASSERT_EQ(sum, VAL3);

    ani_value args[1U];
    args[0U].c = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "nestedMethod", "C:C", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL3);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_char_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    const ani_int value1 = 5;
    const ani_char result1 = 'Z';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "recursiveMethod", "I:C", &sum, value1), ANI_OK);
    ASSERT_EQ(sum, result1);

    const ani_int value2 = -1;
    const ani_char result2 = 'A';
    ani_value args[1U];
    args[0U].i = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "recursiveMethod", "I:C", &sum, args), ANI_OK);
    ASSERT_EQ(sum, result2);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_008)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_char_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    const ani_double dValue1 = 1.0;
    const ani_int iValue1 = 1;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "calculateSum", "CCDI:C", &sum, VAL1, VAL2, dValue1, iValue1),
              ANI_OK);
    ASSERT_EQ(sum, VAL3);

    const ani_double dValue2 = 2.0;
    ani_value args[4U];
    args[0U].c = VAL1;
    args[1U].c = VAL2;
    args[2U].d = dValue2;
    args[3U].i = iValue1;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "calculateSum", "CCDI:C", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1);

    const ani_int iValue2 = 2;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "calculateSum", "CCDI:C", &sum, VAL1, VAL2, dValue2, iValue2),
              ANI_OK);
    ASSERT_EQ(sum, VAL2);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_009)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_char_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    const ani_char value1 = 'D';
    const ani_char value2 = 'S';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "CC:C", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, value2);

    const ani_char value3 = 'D';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "CC:C", &sum, value1, value3), ANI_OK);
    ASSERT_EQ(sum, value3);

    const ani_char value4 = 'Z';
    ani_value args[2U];
    args[0U].c = value1;
    args[1U].c = value4;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "charMethod", "CC:C", &sum, args), ANI_OK);
    ASSERT_EQ(sum, value4);

    const ani_char value5 = 'A';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "CC:C", &sum, value1, value5), ANI_OK);
    ASSERT_EQ(sum, value1);

    const ani_char value6 = 'B';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "CC:C", &sum, value1, value6), ANI_OK);
    ASSERT_EQ(sum, value1);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_010)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_char_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    const ani_char value = 'C';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "jf", "C:C", &sum, value), ANI_OK);
    ASSERT_EQ(sum, value);

    ani_value args[1U];
    args[0U].c = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "jf", "C:C", &sum, args), ANI_OK);
    ASSERT_EQ(sum, value);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)