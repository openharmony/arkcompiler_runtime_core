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

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_a)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].c = 'C';
    args[1U].c = 'A';

    ani_char res;
    char c = 'C' - 'A';
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(object, "char_by_name_method", "CC:C", &res, args), ANI_OK);
    ASSERT_EQ(res, c);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v)
{
    ani_object object;
    GetMethodData(&object);

    ani_int arg1 = 'C';
    ani_int arg2 = 'A';
    ani_char res;
    char c = 'C' - 'A';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "char_by_name_method", "CC:C", &res, arg1, arg2), ANI_OK);
    ASSERT_EQ(res, c);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char)
{
    ani_object object;
    GetMethodData(&object);

    ani_int arg1 = 'D';
    ani_int arg2 = 'A';
    ani_char res;
    char c = 'D' - 'A';
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Char(env_, object, "char_by_name_method", "CC:C", &res, arg1, arg2),
              ANI_OK);
    ASSERT_EQ(res, c);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v_abnormal)
{
    ani_object object;
    GetMethodData(&object);

    ani_int arg1 = 'C';
    ani_int arg2 = 'A';
    ani_char res;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "char_by_name_method", "CC:X", &res, arg1, arg2),
              ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "unknown_function", "CC:C", &res, arg1, arg2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v_invalid_method)
{
    ani_object object;
    GetMethodData(&object);

    ani_int arg1 = 'C';
    ani_int arg2 = 'A';
    ani_char res;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, nullptr, "CC:C", &res, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v_invalid_result)
{
    ani_object object;
    GetMethodData(&object);

    ani_int arg1 = 'C';
    ani_int arg2 = 'A';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "char_by_name_method", "CC:C", nullptr, arg1, arg2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v_invalid_object)
{
    ani_object object;
    GetMethodData(&object);

    ani_int arg1 = 'C';
    ani_int arg2 = 'A';
    ani_char res;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(nullptr, "char_by_name_method", "CC:C", &res, arg1, arg2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_a_invalid_args)
{
    ani_object object;
    GetMethodData(&object);

    ani_char res;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(nullptr, "char_by_name_method", "CC:C", &res, nullptr),
              ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)