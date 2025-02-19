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

class CallObjectMethodByNameDoubleTest : public AniTest {
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

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_a_normal)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    args[0U].d = arg1;
    args[1U].d = arg2;
    ani_double sum;

    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "double_method", "DD:D", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_a_normal_1)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    args[0U].d = arg1;
    args[1U].d = arg2;
    ani_double sum;

    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "double_method", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_a_abnormal)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    args[0U].d = arg1;
    args[1U].d = arg2;
    ani_double sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "xxxxxxx", "DD:D", &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "double_method", "DD:I", &sum, args), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_a_invalid_object)
{
    ani_value args[2U];
    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    args[0U].d = arg1;
    args[1U].d = arg2;
    ani_double sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(nullptr, "double_method", "DD:D", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_a_invalid_method)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    args[0U].d = arg1;
    args[1U].d = arg2;
    ani_double sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, nullptr, "DD:D", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_a_invalid_result)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    args[0U].d = arg1;
    args[1U].d = arg2;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "double_method", "DD:D", nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, cobject_call_method_by_name_double_a_invalid_args)
{
    ani_object object;
    GetMethodData(&object);

    ani_double sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "double_method", "DD:D", &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_normal)
{
    ani_object object;
    GetMethodData(&object);

    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    ani_double sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, "double_method", "DD:D", &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_normal_1)
{
    ani_object object;
    GetMethodData(&object);

    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    ani_double sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, "double_method", nullptr, &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_abnormal)
{
    ani_object object;
    GetMethodData(&object);

    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    ani_double sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, "xxxxxxx", "DD:D", &sum, arg1, arg2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, "double_method", "DD:I", &sum, arg1, arg2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_invalid_object)
{
    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    ani_double sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(nullptr, "double_method", "DD:D", &sum, arg1, arg2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_invalid_method)
{
    ani_object object;
    GetMethodData(&object);

    ani_double sum;
    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, nullptr, "DD:D", &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_invalid_result)
{
    ani_object object;
    GetMethodData(&object);

    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, "double_method", "DD:D", nullptr, arg1, arg2),
              ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)