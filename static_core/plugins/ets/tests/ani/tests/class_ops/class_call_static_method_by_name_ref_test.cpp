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

class ClassCallStaticMethodByNameRefTest : public AniTest {
public:
    static constexpr ani_double VAL1 = 4.5;
    static constexpr ani_double VAL2 = 7.5;
    static constexpr ani_int VAL3 = 5;
    static constexpr ani_int VAL4 = 6;
    void GetMethodData(ani_class *clsResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_ref_test/Phone;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
    void TestFuncV(ani_class cls, const char *name, ani_ref *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref_V(cls, name, "II:Lstd/core/String;", value, args), ANI_OK);
        va_end(args);
    }
    void CheckRefUp(ani_ref ref)
    {
        auto string = reinterpret_cast<ani_string>(ref);
        ani_size result = 0U;
        ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
        ASSERT_EQ(result, 2U);

        ani_size substrOffset = 0U;
        ani_size substrSize = result;
        const uint32_t bufferSize = VAL3;
        char utfBuffer[bufferSize] = {};
        result = 0U;
        auto status =
            env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_STREQ(utfBuffer, "up");
    }
    void CheckRefNum(ani_ref ref)
    {
        auto string = reinterpret_cast<ani_string>(ref);
        ani_size result = 0U;
        ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
        ani_size substrOffset = 0U;
        ani_size substrSize = result;
        const uint32_t bufferSize = 10U;
        char utfBuffer[bufferSize] = {};
        result = 0U;
        auto status =
            env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_STREQ(utfBuffer, "INT5");
    }
};

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_one)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(cls, "get_button_names", nullptr, &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_two)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Ref(env_, cls, "get_button_names", nullptr, &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto string = reinterpret_cast<ani_string>(ref);
    ani_size result = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
    ASSERT_EQ(result, 2U);

    ani_size substrOffset = 0U;
    ani_size substrSize = result;
    const uint32_t bufferSize = VAL3;
    char utfBuffer[bufferSize] = {};
    result = 0U;
    auto status =
        env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_STREQ(utfBuffer, "up");
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_v)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(cls, "get_num_string", nullptr, &ref, VAL3, VAL4), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto string = reinterpret_cast<ani_string>(ref);
    ani_size result = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
    ani_size substrOffset = 0U;
    ani_size substrSize = result;
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {};
    result = 0U;
    auto status =
        env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_STREQ(utfBuffer, "INT5");
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_a)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_ref ref = nullptr;
    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref_A(cls, "get_num_string", nullptr, &ref, args), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto string = reinterpret_cast<ani_string>(ref);
    ani_size result = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
    ani_size substrOffset = 0U;
    ani_size substrSize = result;
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {};
    result = 0U;
    auto status =
        env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_STREQ(utfBuffer, "INT5");
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_invalid_cls)
{
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Ref(env_, nullptr, "get_button_names", nullptr, &ref),
              ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_invalid_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Ref(env_, cls, nullptr, nullptr, &ref), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Ref(env_, cls, "sum_not_exist", nullptr, &ref), ANI_NOT_FOUND);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_invalid_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Ref(env_, cls, "get_button_names", nullptr, nullptr),
              ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_v_invalid_cls)
{
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(nullptr, "get_num_string", nullptr, &ref, VAL3, VAL4),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_v_invalid_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(cls, nullptr, nullptr, &ref, VAL3, VAL4), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(cls, "sum_not_exist", nullptr, &ref, VAL3, VAL4), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_v_invalid_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(cls, "get_num_string", nullptr, nullptr, VAL3, VAL4),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_a_invalid_cls)
{
    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref_A(nullptr, "get_num_string", nullptr, &ref, args),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_a_invalid_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref_A(cls, nullptr, nullptr, &ref, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_a_invalid_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref_A(cls, "get_num_string", nullptr, nullptr, args),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_a_invalid_args)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref_A(cls, "get_num_string", nullptr, &ref, nullptr),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_combine_scenes_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_ref_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ani_ref value = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(cls, "funcA", "II:Lstd/core/String;", &value, VAL3, VAL4), ANI_OK);
    CheckRefNum(value);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_ref valueA = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref_A(cls, "funcA", "II:Lstd/core/String;", &valueA, args), ANI_OK);
    CheckRefNum(valueA);

    ani_ref valueV = nullptr;
    TestFuncV(cls, "funcA", &valueV, VAL3, VAL4);
    CheckRefNum(valueV);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_combine_scenes_2)
{
    ani_namespace nb {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_ref_test/nb;", &nb), ANI_OK);
    ani_namespace nc {};
    ASSERT_EQ(env_->Namespace_FindNamespace(nb, "Lnc;", &nc), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(nc, "LA;", &cls), ANI_OK);

    ani_ref value = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(cls, "funcA", "II:Lstd/core/String;", &value, VAL3, VAL4), ANI_OK);
    CheckRefNum(value);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_ref valueA = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref_A(cls, "funcA", "II:Lstd/core/String;", &valueA, args), ANI_OK);
    CheckRefNum(valueA);

    ani_ref valueV = nullptr;
    TestFuncV(cls, "funcA", &valueV, VAL3, VAL4);
    CheckRefNum(valueV);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_combine_scenes_3)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_ref_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ani_ref value = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(cls, "funcA", "II:Lstd/core/String;", &value, VAL3, VAL4), ANI_OK);
    CheckRefNum(value);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_ref valueA = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref_A(cls, "funcA", "II:Lstd/core/String;", &valueA, args), ANI_OK);
    CheckRefNum(valueA);

    ani_ref valueV = nullptr;
    TestFuncV(cls, "funcA", &valueV, VAL3, VAL4);
    CheckRefNum(valueV);

    ani_double value2 = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "funcA", "DD:D", &value2, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value2, VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameRefTest, call_static_method_by_name_ref_combine_scenes_4)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_ref_test/nd;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LB;", &cls), ANI_OK);

    ani_ref value = nullptr;
    const ani_int value1 = VAL3;
    const ani_int value2 = VAL4;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref(cls, "funcA", "II:Lstd/core/String;", &value, value1, value2),
              ANI_OK);
    CheckRefUp(value);

    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value2;
    ani_ref valueA = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Ref_A(cls, "funcA", "II:Lstd/core/String;", &valueA, args), ANI_OK);
    CheckRefUp(valueA);

    ani_ref valueV = nullptr;
    TestFuncV(cls, "funcA", &valueV, value1, value2);
    CheckRefUp(valueV);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
