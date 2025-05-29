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

class CallObjectMethodByNameVoidTest : public AniTest {
public:
    static constexpr ani_long VAL1 = 1000000;
    static constexpr ani_long VAL2 = 2000000;

    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_void_test/A;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":Lobject_call_method_by_name_void_test/A;", &newMethod),
                  ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);
        *objectResult = static_cast<ani_object>(ref);
    }
};

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_a_normal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long value {};

    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, "voidMethod", "JJ:V", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "getValue", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_a_normal_1)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long value {};

    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, "voidMethod", nullptr, args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "getValue", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_a_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long value {};

    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, "xxxxxxx", "JJ:V", args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "getValue", "JJ:J", &value), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_a_invalid_object)
{
    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(nullptr, "voidMethod", "JJ:V", args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_a_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, nullptr, "JJ:V", args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_a_invalid_args)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, "voidMethod", "JJ:V", nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_normal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long value {};
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "voidMethod", "JJ:V", VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "getValue", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_normal_1)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long value {};
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "voidMethod", nullptr, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "getValue", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long value {};
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "xxxxxxxxx", "JJ:V", VAL1, VAL2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "getValue", "JJ:J", &value), ANI_NOT_FOUND);
}
TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_invalid_object)
{
    ASSERT_EQ(env_->Object_CallMethodByName_Void(nullptr, "voidMethod", "JJ:V", VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, nullptr, "JJ:V", VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_void_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    const ani_int value1 = 5;
    const ani_int value2 = 6;
    ani_int res = 0;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "II:V", value1, value2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value2);

    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "voidMethod", "II:V", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_void_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_char res = 'a';
    const ani_char value = 'D';
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "C:V", value), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "getCharValue", ":C", &res), ANI_OK);
    ASSERT_EQ(res, value);

    ani_value args[1U];
    args[0U].c = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "voidMethod", "C:V", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "getCharValue", ":C", &res), ANI_OK);
    ASSERT_EQ(res, value);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_void_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value1 = 5;
    const ani_int value2 = 8;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidSunMethod", "II:V", value1, value2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value2);

    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "voidSunMethod", "II:V", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_void_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 10;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value1 = 5;
    const ani_int value2 = 5;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "II:V", value1, value2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, arg + value1 + value2);

    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "voidMethod", "II:V", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, arg + value1 + value2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_void_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 5;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value = 5;
    ani_value argsA[1U];
    argsA[0U].i = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "protectedMethod", "I:V", value), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value);
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "protectedMethod", "I:V", argsA), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value);

    ani_double res1 = 0.0;
    const ani_double value1 = 5.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "privateMethod", "D:V", value1), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "getDoubleValue", ":D", &res1), ANI_OK);
    ASSERT_EQ(res1, value1);
    ani_value argsB[1U];
    argsB[0U].d = value1;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "privateMethod", "D:V", argsB), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "getDoubleValue", ":D", &res1), ANI_OK);
    ASSERT_EQ(res1, value1);

    const ani_int value2 = 5;
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_void_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &method), ANI_OK);
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "callProtected", "II:V", value, value2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value + value2);
    ani_value argsC[2U];
    argsC[0U].i = value;
    argsC[1U].i = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "callProtected", "II:V", argsC), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value + value2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_void_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 6;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value = 10;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "nestedMethod", "I:V", value), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, arg + value);

    ani_value args[1U];
    args[0U].i = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "nestedMethod", "I:V", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, arg + value);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_void_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 6;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value1 = 5;
    const ani_int value2 = 15;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "recursiveMethod", "I:V", value1), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value2);

    ani_value args[1U];
    args[0U].i = value1;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "recursiveMethod", "I:V", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value2 + value2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_008)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_void_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 6;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res1 = 0;
    ani_char res2 = 'a';
    ani_double res3 = 0.0;
    const ani_int value1 = 1;
    const ani_char value2 = 'A';
    const ani_double value3 = 1.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "calculateSum", "ICD:V", value1, value2, value3), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res1), ANI_OK);
    ASSERT_EQ(res1, value1);
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "getCharValue", ":C", &res2), ANI_OK);
    ASSERT_EQ(res2, value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "getDoubleValue", ":D", &res3), ANI_OK);
    ASSERT_EQ(res1, value3);

    ani_value args[3U];
    args[0U].i = value1;
    args[1U].c = value2;
    args[2U].c = value3;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "calculateSum", "ICD:V", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res1), ANI_OK);
    ASSERT_EQ(res1, value1);
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "getCharValue", ":C", &res2), ANI_OK);
    ASSERT_EQ(res2, value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "getDoubleValue", ":D", &res3), ANI_OK);
    ASSERT_EQ(res1, value3);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_009)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_void_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    ani_int arg = 15;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value1 = 5;
    const ani_int value2 = 6;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "II:V", value1, value2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value2);

    const ani_int value3 = 7;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "II:V", value1, value3), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value3);

    const ani_int value4 = 3;
    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value4;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "voidMethod", "II:V", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value4);

    const ani_int value5 = 5;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "II:V", value1, value5), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value5);

    const ani_int value6 = 12;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "II:V", value1, value6), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value6);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_010)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_void_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "I:V", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 10;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value = 10;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "jf", "I:V", value), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, arg + value);

    ani_value args[1U];
    args[0U].i = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "jf", "I:V", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":I", &res), ANI_OK);
    ASSERT_EQ(res, arg + value);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)