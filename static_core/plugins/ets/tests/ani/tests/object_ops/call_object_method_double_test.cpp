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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class CallObjectMethodDoubleTest : public AniTest {
public:
    void GetMethodData(ani_object *objectResult, ani_method *methodResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LA;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod;
        ASSERT_EQ(env_->Class_GetStaticMethod(cls, "new_A", ":LA;", &newMethod), ANI_OK);
        ani_ref ref;
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        ani_method method;
        ASSERT_EQ(env_->Class_GetMethod(cls, "double_method", "DD:D", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *objectResult = static_cast<ani_object>(ref);
        *methodResult = method;
    }
};

TEST_F(CallObjectMethodDoubleTest, object_call_method_double_a)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_value args[2U];
    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    args[0U].d = arg1;
    args[1U].d = arg2;

    ani_double sum;
    ASSERT_EQ(env_->Object_CallMethod_Double_A(object, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodDoubleTest, object_call_method_double_v)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_double sum;
    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    ASSERT_EQ(env_->Object_CallMethod_Double(object, method, &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodDoubleTest, object_call_method_double)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_double sum;
    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Double(env_, object, method, &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodDoubleTest, call_method_double_v_invalid_method)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_double sum;
    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    ASSERT_EQ(env_->Object_CallMethod_Double(object, nullptr, &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodDoubleTest, call_method_double_v_invalid_result)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    ASSERT_EQ(env_->Object_CallMethod_Double(object, method, nullptr, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodDoubleTest, call_method_double_v_invalid_object)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_double sum;
    ani_double arg1 = 2.0;
    ani_double arg2 = 3.0;
    ASSERT_EQ(env_->Object_CallMethod_Double(nullptr, method, &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodDoubleTest, call_method_double_a_invalid_args)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_double sum;
    ASSERT_EQ(env_->Object_CallMethod_Double_A(nullptr, method, &sum, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
