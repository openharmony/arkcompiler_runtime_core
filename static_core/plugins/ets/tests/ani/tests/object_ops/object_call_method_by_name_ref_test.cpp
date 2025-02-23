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

class CallObjectMethodByNameRefTest : public AniTest {
public:
    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LA;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":LA;", &newMethod), ANI_OK);
        ani_ref ref;
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        *objectResult = static_cast<ani_object>(ref);
    }
};

TEST_F(CallObjectMethodByNameRefTest, object_call_method_by_name_ref_a_normal)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_int arg1 = 10;
    ani_int arg2 = 20;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Object_CallMethodByName_Ref_A(object, "get_name", nullptr, &ref, args), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto string = reinterpret_cast<ani_string>(ref);
    ani_size result = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
    ASSERT_EQ(result, 8U);

    ani_size substrOffset = 0U;
    ani_size substrSize = result;
    const uint32_t bufferSize = 11U;
    char utfBuffer[bufferSize] = {};
    result = 0U;

    auto status =
        env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_STREQ(utfBuffer, "Equality");
}

TEST_F(CallObjectMethodByNameRefTest, object_call_method_by_name_ref_a_normal_1)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_int arg1 = 11;
    ani_int arg2 = 21;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Object_CallMethodByName_Ref_A(object, "get_name", nullptr, &ref, args), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto string = reinterpret_cast<ani_string>(ref);
    ani_size result = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
    ASSERT_EQ(result, 10U);

    ani_size substrOffset = 0U;
    ani_size substrSize = result;
    const uint32_t bufferSize = 11U;
    char utfBuffer[bufferSize] = {};
    result = 0U;

    auto status =
        env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_STREQ(utfBuffer, "Inequality");
}

TEST_F(CallObjectMethodByNameRefTest, object_call_method_by_name_ref_a_abnormal)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_int arg1 = 10;
    ani_int arg2 = 20;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Object_CallMethodByName_Ref_A(object, "xxxxxx", nullptr, &ref, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Ref_A(object, "get_name", "II:Z", &ref, args), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNameRefTest, object_call_method_by_name_ref_a_invalid_object)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_int arg1 = 10;
    ani_int arg2 = 20;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ani_ref ref = nullptr;

    ASSERT_EQ(env_->Object_CallMethodByName_Ref_A(nullptr, "get_name", nullptr, &ref, args), ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(CallObjectMethodByNameRefTest, object_call_method_by_name_ref_a_invalid_method)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_int arg1 = 10;
    ani_int arg2 = 20;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ani_ref ref = nullptr;

    ASSERT_EQ(env_->Object_CallMethodByName_Ref_A(object, nullptr, nullptr, &ref, args), ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(CallObjectMethodByNameRefTest, object_call_method_by_name_ref_a_invalid_result)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_int arg1 = 10;
    ani_int arg2 = 20;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ani_ref ref = nullptr;

    ASSERT_EQ(env_->Object_CallMethodByName_Ref_A(object, "get_name", nullptr, nullptr, args), ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(CallObjectMethodByNameRefTest, object_call_method_by_name_ref_normal)
{
    ani_object object;
    GetMethodData(&object);

    ani_int arg1 = 10;
    ani_int arg2 = 20;
    ani_ref ref = nullptr;

    ASSERT_EQ(env_->Object_CallMethodByName_Ref(object, "get_name", nullptr, &ref, arg1, arg2), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto string = reinterpret_cast<ani_string>(ref);
    ani_size result = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
    ASSERT_EQ(result, 8U);

    ani_size substrOffset = 0U;
    ani_size substrSize = result;
    const uint32_t bufferSize = 11U;
    char utfBuffer[bufferSize] = {};
    result = 0U;

    auto status =
        env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_STREQ(utfBuffer, "Equality");
}

TEST_F(CallObjectMethodByNameRefTest, object_call_method_by_name_ref_normal_1)
{
    ani_object object;
    GetMethodData(&object);

    ani_int arg1 = 11;
    ani_int arg2 = 20;
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Object_CallMethodByName_Ref(object, "get_name", nullptr, &ref, arg1, arg2), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto string = reinterpret_cast<ani_string>(ref);
    ani_size result = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
    ASSERT_EQ(result, 10U);

    ani_size substrOffset = 0U;
    ani_size substrSize = result;
    const uint32_t bufferSize = 11U;
    char utfBuffer[bufferSize] = {};
    result = 0U;

    auto status =
        env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_STREQ(utfBuffer, "Inequality");
}

TEST_F(CallObjectMethodByNameRefTest, object_call_method_by_name_ref_abnormal)
{
    ani_object object;
    GetMethodData(&object);

    ani_int arg1 = 10;
    ani_int arg2 = 20;
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Object_CallMethodByName_Ref(object, "xxxxxxxx", nullptr, &ref, arg1, arg2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Ref(object, "get_name", "II:J", &ref, arg1, arg2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNameRefTest, object_call_method_by_name_ref_invalid_object)
{
    ani_int arg1 = 10;
    ani_int arg2 = 20;
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Object_CallMethodByName_Ref(nullptr, "get_name", nullptr, &ref, arg1, arg2), ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(CallObjectMethodByNameRefTest, object_call_method_by_name_ref_invalid_method)
{
    ani_object object;
    GetMethodData(&object);

    ani_int arg1 = 10;
    ani_int arg2 = 20;
    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Object_CallMethodByName_Ref(object, nullptr, nullptr, &ref, arg1, arg2), ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(CallObjectMethodByNameRefTest, object_call_method_by_name_ref_invalid_result)
{
    ani_object object;
    GetMethodData(&object);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Object_CallMethodByName_Ref(object, "get_name", nullptr, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)