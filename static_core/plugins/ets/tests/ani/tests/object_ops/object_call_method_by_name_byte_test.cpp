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

class CallObjectMethodByteByNameTest : public AniTest {
public:
    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls;
        // Locate the class "LA;" in the environment.
        ASSERT_EQ(env_->FindClass("LA;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":LA;", &newMethod), ANI_OK);
        ani_ref ref;
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);
        *objectResult = static_cast<ani_object>(ref);
    }
};

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_a)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].b = 5U;
    args[1U].b = 6U;

    ani_byte res;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte_A(object, "byte_by_name_method", "BB:B", &res, args), ANI_OK);
    ASSERT_EQ(res, 5U + 6U);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_v)
{
    ani_object object;
    GetMethodData(&object);

    ani_byte res;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(object, "byte_by_name_method", "BB:B", &res, 5U, 6U), ANI_OK);
    ASSERT_EQ(res, 5U + 6U);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte)
{
    ani_object object;
    GetMethodData(&object);

    ani_byte res;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Byte(env_, object, "byte_by_name_method", "BB:B", &res, 2U, 6U),
              ANI_OK);
    ASSERT_EQ(res, 2U + 6U);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_v_abnormal)
{
    ani_object object;
    GetMethodData(&object);

    ani_byte res;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(object, "byte_by_name_method", "BB:X", &res, 5U, 6U), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(object, "unknown_function", "BB:B", &res, 5U, 6U), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_v_invalid_method)
{
    ani_object object;
    GetMethodData(&object);

    ani_byte res;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(object, nullptr, "BB:B", &res, 5U, 6U), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_v_invalid_result)
{
    ani_object object;
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Byte(object, "byte_by_name_method", "BB:B", nullptr, 5U, 6U),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_v_invalid_object)
{
    ani_object object;
    GetMethodData(&object);

    ani_byte res;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(nullptr, "byte_by_name_method", "BB:B", &res, 5U, 6U),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByteByNameTest, object_call_method_byte_a_invalid_args)
{
    ani_object object;
    GetMethodData(&object);

    ani_byte res;
    ASSERT_EQ(env_->Object_CallMethodByName_Byte(nullptr, "byte_by_name_method", "BB:B", &res, nullptr),
              ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)