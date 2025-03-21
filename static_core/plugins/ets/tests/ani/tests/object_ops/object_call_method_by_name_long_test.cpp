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

class CallObjectMethodByNamelongTest : public AniTest {
public:
    static constexpr ani_long VAL0 = 1000000;
    static constexpr ani_long VAL1 = 5000;
    static constexpr ani_long VAL2 = 6000;

    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_long_test/A;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":Lobject_call_method_by_name_long_test/A;", &newMethod),
                  ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        *objectResult = static_cast<ani_object>(ref);
    }
};

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_normal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", "JJ:J", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_normal_1)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "xxxxxxxxxx", "JJ:J", &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", "JJ:I", &sum, args), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_invalid_object)
{
    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(nullptr, "longMethod", "JJ:J", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, nullptr, "JJ:J", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", "JJ:J", nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, cobject_call_method_by_name_long_a_invalid_args)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", "JJ:J", &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_normal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", "JJ:J", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_normal_1)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", nullptr, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "XXXXXXX", "JJ:J", &sum, VAL1, VAL2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", "JJ:I", &sum, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_invalid_object)
{
    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(nullptr, "longMethod", "JJ:J", &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, nullptr, "JJ:J", &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", "JJ:J", nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_long_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "J:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "JJ:J", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL0);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longMethod", "JJ:J", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_long_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "J:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "J:J", &sum, VAL1), ANI_OK);
    ASSERT_EQ(sum, VAL0);

    ani_value args[1U];
    args[0U].l = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longMethod", "J:J", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_long_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "J:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_long sum {};
    const ani_long value1 = 50000;
    const ani_long value2 = 80000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longAddMethod", "JJ:J", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, VAL0 + value1 + value2);

    ani_value args[2U];
    args[0U].l = value1;
    args[1U].l = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longAddMethod", "JJ:J", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0 + value1 + value2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_long_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "J:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_long sum {};
    const ani_long value1 = 50000;
    const ani_long value2 = 80000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "JJ:J", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, VAL0 - value1 - value2);

    ani_value args[2U];
    args[0U].l = value1;
    args[1U].l = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longMethod", "JJ:J", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0 - value1 - value2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_long_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "J:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_long num {};
    ani_value args[1U];
    args[0U].l = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "protectedMethod", "J:J", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "protectedMethod", "J:J", &num, args), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);

    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "privateMethod", "J:J", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, VAL0 - VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "privateMethod", "J:J", &num, args), ANI_OK);
    ASSERT_EQ(num, VAL0 - VAL1);

    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_long_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "J:V", &method), ANI_OK);
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "callProtected", "J:J", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "callProtected", "J:J", &num, args), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_long_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "J:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "nestedMethod", "J:J", &sum, VAL1), ANI_OK);
    ASSERT_EQ(sum, VAL2 + VAL1);

    ani_value args[1U];
    args[0U].l = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "nestedMethod", "J:J", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL2 + VAL1);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_long_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "J:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_long sum {};
    const ani_int value1 = 5;
    const ani_long result = 120;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "recursiveMethod", "I:J", &sum, value1), ANI_OK);
    ASSERT_EQ(sum, result);

    ani_value args[1U];
    args[0U].i = value1;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "recursiveMethod", "I:J", &sum, args), ANI_OK);
    ASSERT_EQ(sum, result);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_008)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_long_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "J:V", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_long sum {};
    const ani_long value1 = 1000;
    const ani_char value2 = 'A';
    const ani_int value3 = 1;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "calculateSum", "JCI:J", &sum, value1, value2, value3), ANI_OK);
    ASSERT_EQ(sum, VAL2 - value1);

    const ani_char value4 = 'B';
    ani_value args[3U];
    args[0U].l = value1;
    args[1U].c = value4;
    args[2U].i = value3;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "calculateSum", "JCI:J", &sum, args), ANI_OK);
    ASSERT_EQ(sum, value1);

    const ani_int value5 = 2U;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "calculateSum", "JCI:J", &sum, value1, value4, value5), ANI_OK);
    ASSERT_EQ(sum, VAL2 + value1);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_009)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_long_test/B;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "J:V", &method), ANI_OK);

    ani_object obj {};
    const ani_long arg = 15000;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_long sum {};
    const ani_long value1 = 5000;
    const ani_long value2 = 6000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "JJ:J", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_long value3 = 7000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "JJ:J", &sum, value1, value3), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_long value4 = 3000;
    ani_value args[2U];
    args[0U].l = value1;
    args[1U].l = value4;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longMethod", "JJ:J", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_long value5 = 10000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "JJ:J", &sum, value1, value5), ANI_OK);
    ASSERT_EQ(sum, value1 + value5);

    const ani_long value6 = 12000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "JJ:J", &sum, value1, value6), ANI_OK);
    ASSERT_EQ(sum, value1 + value6);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_010)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lobject_call_method_by_name_long_test/C;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "J:V", &method), ANI_OK);

    ani_object obj {};
    ani_long arg = 1000;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_long sum {};
    const ani_long value = 200;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "jf", "J:J", &sum, value), ANI_OK);
    ASSERT_EQ(sum, arg + value);

    ani_value args[1U];
    args[0U].l = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "jf", "J:J", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg + value);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_011)
{
    ani_object obj {};
    GetMethodData(&obj);

    ani_long sum = 0L;
    const ani_long value1 = std::numeric_limits<ani_long>::max();
    const ani_long value2 = 0L;
    ani_value args1[2U];
    args1[0U].l = value1;
    args1[1U].l = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "JJ:J", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longMethod", "JJ:J", &sum, args1), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);

    const ani_long value3 = std::numeric_limits<ani_long>::min();
    ani_value args2[2U];
    args2[0U].l = value3;
    args2[1U].l = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "JJ:J", &sum, value3, value2), ANI_OK);
    ASSERT_EQ(sum, value3 + value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longMethod", "JJ:J", &sum, args2), ANI_OK);
    ASSERT_EQ(sum, value3 + value2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_012)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long res = 0;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Long(nullptr, object, "longMethod", "JJ:J", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Long_A(nullptr, object, "longMethod", "JJ:J", &res, args),
              ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Long(nullptr, "longMethod", "JJ:J", &res, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(nullptr, "longMethod", "JJ:J", &res, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, nullptr, "JJ:J", &res, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, nullptr, "JJ:J", &res, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", nullptr, &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", nullptr, &res, args), ANI_OK);

    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", "JJ:J", nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", "JJ:J", nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_013)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long res = 0;
    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Long(object, methodName.data(), "JJ:J", &res, VAL1, VAL2),
                  ANI_NOT_FOUND);
        ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, methodName.data(), "JJ:J", &res, args), ANI_NOT_FOUND);
    }
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_014)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long res = 0;
    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", methodName.data(), &res, VAL1, VAL2),
                  ANI_NOT_FOUND);
        ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", methodName.data(), &res, args),
                  ANI_NOT_FOUND);
    }
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)