/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

class CallStaticMethodTest : public AniTest {
public:
    void GetMethodDataButton(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LPhone;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_GetStaticMethod(cls, "get_button_names", nullptr, &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }

    void GetMethodDataString(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LPhone;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_GetStaticMethod(cls, "get_num_string", "II:Lstd/core/String;", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }
};

TEST_F(CallStaticMethodTest, call_static_method_ref_0)
{
    ani_class cls = nullptr;
    ani_static_method method = nullptr;
    GetMethodDataButton(&cls, &method);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method, &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);
}

TEST_F(CallStaticMethodTest, call_static_method_ref)
{
    ani_class cls = nullptr;
    ani_static_method method = nullptr;
    GetMethodDataButton(&cls, &method);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Ref(env_, cls, method, &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto string = reinterpret_cast<ani_string>(ref);
    ani_size result = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
    ASSERT_EQ(result, 2U);

    ani_size substrOffset = 0U;
    ani_size substrSize = result;
    const uint32_t bufferSize = 5U;
    char utfBuffer[bufferSize] = {};
    result = 0U;
    auto status =
        env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_STREQ(utfBuffer, "up");
}

TEST_F(CallStaticMethodTest, call_static_method_ref_v)
{
    ani_class cls = nullptr;
    ani_static_method method = nullptr;
    GetMethodDataString(&cls, &method);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method, &ref, 5U, 6U), ANI_OK);
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

TEST_F(CallStaticMethodTest, call_static_method_ref_a)
{
    ani_class cls = nullptr;
    ani_static_method method = nullptr;
    GetMethodDataString(&cls, &method);

    ani_ref ref = nullptr;
    ani_value args[2U];
    args[0U].i = 5U;
    args[1U].i = 6U;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, method, &ref, args), ANI_OK);
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

TEST_F(CallStaticMethodTest, call_static_method_ref_invalid_cls)
{
    ani_class cls = nullptr;
    ani_static_method method = nullptr;
    GetMethodDataButton(&cls, &method);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Ref(env_, nullptr, method, &ref), ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_invalid_method)
{
    ani_class cls = nullptr;
    ani_static_method method = nullptr;
    GetMethodDataButton(&cls, &method);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Ref(env_, cls, nullptr, &ref), ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_invalid_result)
{
    ani_class cls = nullptr;
    ani_static_method method = nullptr;
    GetMethodDataButton(&cls, &method);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Ref(env_, cls, method, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_v_invalid_cls)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ani_ref ref;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(nullptr, method, &ref, 5U, 6U), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_v_invalid_method)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ani_ref ref;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, nullptr, &ref, 5U, 6U), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_v_invalid_result)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method, nullptr, 5U, 6U), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_a_invalid_cls)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ani_value args[2U];
    args[0U].i = 5U;
    args[1U].i = 6U;
    ani_ref ref;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(nullptr, method, &ref, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_a_invalid_method)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ani_value args[2U];
    args[0U].i = 5U;
    args[1U].i = 6U;
    ani_ref ref;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, nullptr, &ref, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_a_invalid_result)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ani_value args[2U];
    args[0U].i = 5U;
    args[1U].i = 6U;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, method, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_a_invalid_args)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ani_ref ref;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(nullptr, method, &ref, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)