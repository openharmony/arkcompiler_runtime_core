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

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_normal)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_long arg1 = 1000000;
    ani_long arg2 = 2000000;
    args[0U].l = arg1;
    args[1U].l = arg2;
    ani_long sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "long_method", "JJ:J", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_normal_1)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_long arg1 = 1000000;
    ani_long arg2 = 2000000;
    args[0U].l = arg1;
    args[1U].l = arg2;
    ani_long sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "long_method", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_abnormal)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_long arg1 = 1000000;
    ani_long arg2 = 2000000;
    args[0U].l = arg1;
    args[1U].l = arg2;
    ani_long sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "xxxxxxxxxx", "JJ:J", &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "long_method", "JJ:I", &sum, args), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_invalid_object)
{
    ani_value args[2U];
    ani_long arg1 = 1000000;
    ani_long arg2 = 2000000;
    args[0U].l = arg1;
    args[1U].l = arg2;
    ani_long sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(nullptr, "long_method", "JJ:J", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_invalid_method)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_long arg1 = 1000000;
    ani_long arg2 = 2000000;
    args[0U].l = arg1;
    args[1U].l = arg2;
    ani_long sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, nullptr, "JJ:J", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_invalid_result)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    ani_long arg1 = 1000000;
    ani_long arg2 = 2000000;
    args[0U].l = arg1;
    args[1U].l = arg2;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "long_method", "JJ:J", nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, cobject_call_method_by_name_long_a_invalid_args)
{
    ani_object object;
    GetMethodData(&object);

    ani_long sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "long_method", "JJ:J", &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_normal)
{
    ani_object object;
    GetMethodData(&object);

    ani_long arg1 = 1000000;
    ani_long arg2 = 2000000;
    ani_long sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "long_method", "JJ:J", &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_normal_1)
{
    ani_object object;
    GetMethodData(&object);

    ani_long arg1 = 1000000;
    ani_long arg2 = 2000000;
    ani_long sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "long_method", nullptr, &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_abnormal)
{
    ani_object object;
    GetMethodData(&object);

    ani_long arg1 = 1000000;
    ani_long arg2 = 2000000;
    ani_long sum;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "XXXXXXX", "JJ:J", &sum, arg1, arg2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "long_method", "JJ:I", &sum, arg1, arg2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_invalid_object)
{
    ani_long sum;
    ani_long arg1 = 1000000;
    ani_long arg2 = 2000000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(nullptr, "long_method", "JJ:J", &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_invalid_method)
{
    ani_object object;
    GetMethodData(&object);

    ani_long sum;
    ani_long arg1 = 1000000;
    ani_long arg2 = 2000000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, nullptr, "JJ:J", &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_invalid_result)
{
    ani_object object;
    GetMethodData(&object);

    ani_long arg1 = 1000000;
    ani_long arg2 = 2000000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "long_method", "JJ:J", nullptr, arg1, arg2), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)