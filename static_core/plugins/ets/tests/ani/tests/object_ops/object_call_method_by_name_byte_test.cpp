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

class CallObjectMethodByteByNameTest : public AniTest {
public:
    static constexpr ani_byte VAL1 = 5U;
    static constexpr ani_byte VAL2 = 6U;

    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls {};
        // Locate the class "LA;" in the environment.
        ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_byte_test/A;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":Lobject_call_method_by_name_byte_test/A;", &newMethod),
                  ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);
        *objectResult = static_cast<ani_object>(ref);
    }
};

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_a)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;

    ani_byte res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(object, "byteByNameMethod", "BB:B", &res, args), ANI_OK);
    ASSERT_EQ(res, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_v)
{
    ani_object object {};
    GetMethodData(&object);

    ani_byte res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(object, "byteByNameMethod", "BB:B", &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(res, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte)
{
    ani_object object {};
    GetMethodData(&object);

    ani_byte res {};
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Byte(env_, object, "byteByNameMethod", "BB:B", &res, VAL1, VAL2),
              ANI_OK);
    ASSERT_EQ(res, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_v_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_byte res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(object, "byteByNameMethod", "BB:X", &res, VAL1, VAL2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(object, "unknown_function", "BB:B", &res, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_v_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_byte res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(object, nullptr, "BB:B", &res, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_v_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Byte(object, "byteByNameMethod", "BB:B", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_v_invalid_object)
{
    ani_object object {};
    GetMethodData(&object);

    ani_byte res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(nullptr, "byteByNameMethod", "BB:B", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_a_invalid_args)
{
    ani_object object {};
    GetMethodData(&object);

    ani_byte res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(nullptr, "byteByNameMethod", "BB:B", &res, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_by_name_byte_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_byte_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "B:V", &method), ANI_OK);

    ani_object obj {};
    const ani_byte arg = 100U;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_byte sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "byteMethod", "BB:B", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, arg);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(obj, "byteMethod", "BB:B", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_by_name_byte_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_byte_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "B:V", &method), ANI_OK);

    ani_object obj {};
    const ani_byte arg = 100U;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_byte sum {};
    const ani_byte value = 5U;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "byteMethod", "B:B", &sum, value), ANI_OK);
    ASSERT_EQ(sum, arg);

    ani_value args[1U];
    args[0U].b = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(obj, "byteMethod", "B:B", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_by_name_byte_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_byte_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "B:V", &method), ANI_OK);

    ani_object obj {};
    const ani_byte arg = 100U;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_byte sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "byteAddMethod", "BB:B", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, arg + VAL1 + VAL2);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(obj, "byteAddMethod", "BB:B", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg + VAL1 + VAL2);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_by_name_byte_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_byte_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "B:V", &method), ANI_OK);

    ani_object obj {};
    const ani_byte arg = 100U;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_byte sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "byteMethod", "BB:B", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, arg - VAL1 - VAL2);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(obj, "byteMethod", "BB:B", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg - VAL1 - VAL2);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_by_name_byte_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_byte_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "B:V", &method), ANI_OK);

    ani_object obj {};
    const ani_byte arg = 100U;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_byte num {};
    ani_value args[1U];
    args[0U].b = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "protectedMethod", "B:B", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, arg + VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(obj, "protectedMethod", "B:B", &num, args), ANI_OK);
    ASSERT_EQ(num, arg + VAL1);

    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "privateMethod", "B:B", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, arg - VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(obj, "privateMethod", "B:B", &num, args), ANI_OK);
    ASSERT_EQ(num, arg - VAL1);

    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_byte_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "B:V", &method), ANI_OK);
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "callProtected", "B:B", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, arg + VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(obj, "callProtected", "B:B", &num, args), ANI_OK);
    ASSERT_EQ(num, arg + VAL1);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_by_name_byte_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_byte_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "B:V", &method), ANI_OK);

    ani_object obj {};
    const ani_byte arg = 6U;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_byte sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "nestedMethod", "B:B", &sum, VAL1), ANI_OK);
    ASSERT_EQ(sum, arg + VAL1);

    ani_value args[1U];
    args[0U].b = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(obj, "nestedMethod", "B:B", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg + VAL1);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_by_name_byte_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_byte_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "B:V", &method), ANI_OK);

    ani_object obj {};
    const ani_byte arg = 6U;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_byte sum {};
    const ani_int value1 = 5;
    const ani_byte result = 120;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "recursiveMethod", "I:B", &sum, value1), ANI_OK);
    ASSERT_EQ(sum, result);

    ani_value args[1U];
    args[0U].i = value1;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(obj, "recursiveMethod", "I:B", &sum, args), ANI_OK);
    ASSERT_EQ(sum, result);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_by_name_byte_008)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_byte_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "B:V", &method), ANI_OK);

    ani_object obj {};
    const ani_byte arg = 6U;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_byte sum {};
    const ani_byte value = 1U;
    const ani_char cValue1 = 'A';
    const ani_int iValue1 = 1;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "calculateSum", "BCI:B", &sum, value, cValue1, iValue1), ANI_OK);
    ASSERT_EQ(sum, arg - value);

    const ani_char cValue2 = 'B';
    ani_value args[3U];
    args[0U].b = value;
    args[1U].c = cValue2;
    args[2U].i = iValue1;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(obj, "calculateSum", "BCI:B", &sum, args), ANI_OK);
    ASSERT_EQ(sum, value);

    const ani_int iValue2 = 2U;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "calculateSum", "BCI:B", &sum, value, cValue2, iValue2), ANI_OK);
    ASSERT_EQ(sum, arg + value);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_by_name_byte_009)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_byte_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "B:V", &method), ANI_OK);

    ani_object obj {};
    const ani_byte arg = 15U;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_byte sum {};
    const ani_byte value1 = 5U;
    const ani_byte value2 = 6U;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "byteMethod", "BB:B", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_byte value3 = 7U;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "byteMethod", "BB:B", &sum, value1, value3), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_byte value4 = 3U;
    ani_value args[2U];
    args[0U].b = value1;
    args[1U].b = value4;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(obj, "byteMethod", "BB:B", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_byte value5 = 10U;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "byteMethod", "BB:B", &sum, value1, value5), ANI_OK);
    ASSERT_EQ(sum, value1 + value5);

    const ani_byte value6 = 12U;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "byteMethod", "BB:B", &sum, value1, value6), ANI_OK);
    ASSERT_EQ(sum, value1 + value6);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_by_name_byte_010)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_byte_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "B:V", &method), ANI_OK);

    ani_object obj {};
    const ani_byte arg = 10U;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_byte sum {};
    const ani_byte value = 2U;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(obj, "jf", "B:B", &sum, value), ANI_OK);
    ASSERT_EQ(sum, arg + value);

    ani_value args[1U];
    args[0U].b = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(obj, "jf", "B:B", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg + value);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)