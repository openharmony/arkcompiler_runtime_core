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

class CallObjectMethodIntByNameTest : public AniTest {
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

TEST_F(CallObjectMethodIntByNameTest, object_call_method_int_a)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].i = 5U;
    args[1U].i = 6U;

    ani_int res;
    ASSERT_EQ(env_->Object_CallMethodByName_Int_A(object, "int_by_name_method", "II:I", &res, args), ANI_OK);
    ASSERT_EQ(res, 5U + 6U);
}

TEST_F(CallObjectMethodIntByNameTest, object_call_method_int_v)
{
    ani_object object;
    GetMethodData(&object);

    ani_int res;
    ASSERT_EQ(env_->Object_CallMethodByName_Int(object, "int_by_name_method", "II:I", &res, 5U, 6U), ANI_OK);
    ASSERT_EQ(res, 5U + 6U);
}

TEST_F(CallObjectMethodIntByNameTest, object_call_method_int)
{
    ani_object object;
    GetMethodData(&object);

    ani_int res;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Int(env_, object, "int_by_name_method", "II:I", &res, 2U, 6U),
              ANI_OK);
    ASSERT_EQ(res, 2U + 6U);
}

TEST_F(CallObjectMethodIntByNameTest, object_call_method_int_v_abnormal)
{
    ani_object object;
    GetMethodData(&object);

    ani_int res;
    ASSERT_EQ(env_->Object_CallMethodByName_Int(object, "int_by_name_method", "II:X", &res, 5U, 6U), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(object, "unknown_function", "II:I", &res, 5U, 6U), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodIntByNameTest, object_call_method_int_v_invalid_method)
{
    ani_object object;
    GetMethodData(&object);

    ani_int res;
    ASSERT_EQ(env_->Object_CallMethodByName_Int(object, nullptr, "II:I", &res, 5U, 6U), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodIntByNameTest, object_call_method_int_v_invalid_result)
{
    ani_object object;
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Int(object, "int_by_name_method", "II:I", nullptr, 5U, 6U),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodIntByNameTest, object_call_method_int_v_invalid_object)
{
    ani_object object;
    GetMethodData(&object);

    ani_int res;
    ASSERT_EQ(env_->Object_CallMethodByName_Int(nullptr, "int_by_name_method", "II:I", &res, 5U, 6U), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodIntByNameTest, object_call_method_int_a_invalid_args)
{
    ani_object object;
    GetMethodData(&object);

    ani_int res;
    ASSERT_EQ(env_->Object_CallMethodByName_Int(nullptr, "int_by_name_method", "II:I", &res, nullptr),
              ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)