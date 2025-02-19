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

class CallObjectMethodBooleanByNameTest : public AniTest {
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

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_boolean_a)
{
    ani_object object;
    GetMethodData(&object);

    ani_value args[2];
    ani_int arg1 = 2U;
    ani_int arg2 = 3U;
    args[0].i = arg1;
    args[1].i = arg2;

    ani_boolean res;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(object, "boolean_by_name_method", "II:Z", &res, args), ANI_OK);
    ASSERT_EQ(res, false);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_boolean_v)
{
    ani_object object;
    GetMethodData(&object);

    ani_boolean res;
    ani_int arg1 = 5U;
    ani_int arg2 = 6U;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "boolean_by_name_method", "II:Z", &res, arg1, arg2),
              ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_boolean_v_abnormal)
{
    ani_object object;
    GetMethodData(&object);

    ani_boolean res;
    ani_int arg1 = 5U;
    ani_int arg2 = 6U;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "boolean_by_name_method", "II:X", &res, arg1, arg2),
              ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "unknown_function", "II:Z", &res, arg1, arg2),
              ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_boolean)
{
    ani_object object;
    GetMethodData(&object);

    ani_boolean res;
    ani_int arg1 = 2U;
    ani_int arg2 = 3U;
    ASSERT_EQ(
        env_->c_api->Object_CallMethodByName_Boolean(env_, object, "boolean_by_name_method", "II:Z", &res, arg1, arg2),
        ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanByNameTest, call_method_boolean_v_invalid_method)
{
    ani_object object;
    GetMethodData(&object);

    ani_boolean res;
    ani_int arg1 = 5U;
    ani_int arg2 = 6U;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, nullptr, "II:Z", &res, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanByNameTest, call_method_boolean_v_invalid_result)
{
    ani_object object;
    GetMethodData(&object);

    ani_int arg1 = 5U;
    ani_int arg2 = 6U;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "boolean_by_name_method", "II:Z", nullptr, arg1, arg2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanByNameTest, call_method_boolean_v_invalid_object)
{
    ani_object object;
    GetMethodData(&object);

    ani_boolean res;
    ani_int arg1 = 5U;
    ani_int arg2 = 6U;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(nullptr, "boolean_by_name_method", "II:Z", &res, arg1, arg2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanByNameTest, call_method_boolean_a_invalid_args)
{
    ani_object object;
    GetMethodData(&object);

    ani_boolean res;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(nullptr, "boolean_by_name_method", "II:Z", &res, nullptr),
              ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)