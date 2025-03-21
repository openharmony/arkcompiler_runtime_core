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

class CallObjectMethodShortTest : public AniTest {
public:
    void GetMethodData(ani_object *objectResult, ani_method *methodResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("Lcall_object_method_short_test/A;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":Lcall_object_method_short_test/A;", &newMethod), ANI_OK);
        ani_ref ref;
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        ani_method method;
        ASSERT_EQ(env_->Class_FindMethod(cls, "short_method", "SS:S", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *objectResult = static_cast<ani_object>(ref);
        *methodResult = method;
    }
};

TEST_F(CallObjectMethodShortTest, object_call_method_short_a)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_value args[2U];
    ani_short arg1 = 2U;
    ani_short arg2 = 3U;
    args[0U].s = arg1;
    args[1U].s = arg2;

    ani_short sum;
    ASSERT_EQ(env_->Object_CallMethod_Short_A(object, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg2);
}

TEST_F(CallObjectMethodShortTest, object_call_method_short_v)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_short sum;
    ani_short arg1 = 2U;
    ani_short arg2 = 3U;
    ASSERT_EQ(env_->Object_CallMethod_Short(object, method, &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg2);
}

TEST_F(CallObjectMethodShortTest, object_call_method_short)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_short sum;
    ani_short arg1 = 4U;
    ani_short arg2 = 3U;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Short(env_, object, method, &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg1);
}

TEST_F(CallObjectMethodShortTest, call_method_short_v_invalid_method)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_short sum;
    ani_short arg1 = 5U;
    ani_short arg2 = 6U;
    ASSERT_EQ(env_->Object_CallMethod_Short(object, nullptr, &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortTest, call_method_short_v_invalid_result)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_short arg1 = 5U;
    ani_short arg2 = 6U;
    ASSERT_EQ(env_->Object_CallMethod_Short(object, method, nullptr, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortTest, call_method_short_v_invalid_object)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_short sum;
    ani_short arg1 = 5U;
    ani_short arg2 = 6U;
    ASSERT_EQ(env_->Object_CallMethod_Short(nullptr, method, &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortTest, call_method_short_a_invalid_args)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_short sum;
    ASSERT_EQ(env_->Object_CallMethod_Short_A(object, method, &sum, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
