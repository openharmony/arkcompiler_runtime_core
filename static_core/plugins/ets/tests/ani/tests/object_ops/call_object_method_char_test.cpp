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

class CallObjectMethodCharTest : public AniTest {
public:
    void GetMethodData(ani_object *objectResult, ani_method *methodResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LA;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":LA;", &newMethod), ANI_OK);
        ani_ref ref;
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        ani_method method;
        ASSERT_EQ(env_->Class_FindMethod(cls, "char_method", "CC:C", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *objectResult = static_cast<ani_object>(ref);
        *methodResult = method;
    }
};

TEST_F(CallObjectMethodCharTest, object_call_method_char_a)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_value args[2U];
    ani_char arg1 = 'a';
    ani_char arg2 = 'b';
    args[0U].c = arg1;
    args[1U].c = arg2;

    ani_char result;
    ASSERT_EQ(env_->Object_CallMethod_Char_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, arg2);
}

TEST_F(CallObjectMethodCharTest, call_method_char_a_invalid_args)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_char result;
    ASSERT_EQ(env_->Object_CallMethod_Char_A(object, method, &result, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharTest, object_call_method_char_v)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_char result;
    ani_char arg1 = 'a';
    ani_char arg2 = 'b';
    ASSERT_EQ(env_->Object_CallMethod_Char(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, arg2);
}

TEST_F(CallObjectMethodCharTest, object_call_method_char)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_char result;
    ani_char arg1 = 'a';
    ani_char arg2 = 'b';
    ASSERT_EQ(env_->c_api->Object_CallMethod_Char(env_, object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, arg2);
}

TEST_F(CallObjectMethodCharTest, call_method_char_v_invalid_object)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_char result;
    ani_char arg1 = 'a';
    ani_char arg2 = 'b';
    ASSERT_EQ(env_->Object_CallMethod_Char(nullptr, method, &result, arg1, arg2), ANI_INVALID_ARGS);
}
TEST_F(CallObjectMethodCharTest, call_method_char_v_invalid_method)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_char result;
    ani_char arg1 = 'a';
    ani_char arg2 = 'b';
    ASSERT_EQ(env_->Object_CallMethod_Char(object, nullptr, &result, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharTest, call_method_char_v_invalid_result)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_char arg1 = 'a';
    ani_char arg2 = 'b';
    ASSERT_EQ(env_->Object_CallMethod_Char(object, method, nullptr, arg1, arg2), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
