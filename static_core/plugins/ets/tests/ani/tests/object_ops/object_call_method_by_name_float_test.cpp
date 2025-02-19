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

class CallObjectMethodByNamefloatTest : public AniTest {
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

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_a_normal)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_float arg1 = 2.0;
    ani_float arg2 = 3.0;
    args[0U].f = arg1;
    args[1U].f = arg2;
    ani_float sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "float_method", "FF:F", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_a_normal_1)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_float arg1 = 2.0;
    ani_float arg2 = 3.0;
    args[0U].f = arg1;
    args[1U].f = arg2;
    ani_float sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "float_method", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_a_abnormal)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_float arg1 = 2.0;
    ani_float arg2 = 3.0;
    args[0U].f = arg1;
    args[1U].f = arg2;
    ani_float sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "xxxxxxxxx", "FF:F", &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "float_method", "FF:I", &sum, args), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_a_invalid_object)
{
    ani_value args[2U];
    ani_float arg1 = 2.0;
    ani_float arg2 = 3.0;
    args[0U].f = arg1;
    args[1U].f = arg2;
    ani_float sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(nullptr, "float_method", "FF:F", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_a_invalid_method)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_float arg1 = 2.0;
    ani_float arg2 = 3.0;
    args[0U].f = arg1;
    args[1U].f = arg2;
    ani_float sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, nullptr, "FF:F", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_a_invalid_result)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_float arg1 = 2.0;
    ani_float arg2 = 3.0;
    args[0U].f = arg1;
    args[1U].f = arg2;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "float_method", "FF:F", nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, cobject_call_method_by_name_float_a_invalid_args)
{
    ani_object object;
    GetMethodData(&object);

    ani_float sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "float_method", "FF:F", &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_normal)
{
    ani_object object;
    GetMethodData(&object);

    ani_float arg1 = 2.0;
    ani_float arg2 = 3.0;
    ani_float sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, "float_method", "FF:F", &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_normal_1)
{
    ani_object object;
    GetMethodData(&object);

    ani_float arg1 = 2.0;
    ani_float arg2 = 3.0;
    ani_float sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, "float_method", nullptr, &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_abnormal)
{
    ani_object object;
    GetMethodData(&object);

    ani_float arg1 = 2.0;
    ani_float arg2 = 3.0;
    ani_float sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, "xxxxxxxxxx", "FF:F", &sum, arg1, arg2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, "float_method", "FF:I", &sum, arg1, arg2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_invalid_object)
{
    ani_float sum;
    ani_float arg1 = 2.0;
    ani_float arg2 = 3.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(nullptr, "float_method", "FF:F", &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_invalid_method)
{
    ani_object object;
    GetMethodData(&object);

    ani_float sum;
    ani_float arg1 = 2.0;
    ani_float arg2 = 3.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, nullptr, "FF:F", &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_invalid_result)
{
    ani_object object;
    GetMethodData(&object);

    ani_float arg1 = 2.0;
    ani_float arg2 = 3.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, "float_method", "FF:F", nullptr, arg1, arg2),
              ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)