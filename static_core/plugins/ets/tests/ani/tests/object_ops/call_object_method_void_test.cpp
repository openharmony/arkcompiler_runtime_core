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

class CallObjectMethodVoidTest : public AniTest {
public:
    void GetMethodData(ani_object *objectResult, ani_method *voidMethodResult, ani_method *getMethodResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("Lcall_object_method_void_test/A;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":Lcall_object_method_void_test/A;", &newMethod), ANI_OK);
        ani_ref ref;
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        ani_method getMethod;
        ASSERT_EQ(env_->Class_FindMethod(cls, "getCount", nullptr, &getMethod), ANI_OK);
        ASSERT_NE(getMethod, nullptr);

        ani_method voidMethod;
        ASSERT_EQ(env_->Class_FindMethod(cls, "voidMethod", nullptr, &voidMethod), ANI_OK);
        ASSERT_NE(voidMethod, nullptr);

        *objectResult = static_cast<ani_object>(ref);
        *voidMethodResult = voidMethod;
        *getMethodResult = getMethod;
    }
};

TEST_F(CallObjectMethodVoidTest, object_call_method_void_a)
{
    ani_object object;
    ani_method voidMethod;
    ani_method getMethod;
    GetMethodData(&object, &voidMethod, &getMethod);

    ani_value args[2];
    ani_int arg1 = 2;
    ani_int arg2 = 3;
    args[0].i = arg1;
    args[1].i = arg2;

    ani_int sum;
    ASSERT_EQ(env_->Object_CallMethod_Void_A(object, voidMethod, args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, getMethod, &sum), ANI_OK);
    ASSERT_EQ(sum, args[0].i + args[1].i);
}

TEST_F(CallObjectMethodVoidTest, object_call_method_void_v)
{
    ani_object object;
    ani_method voidMethod;
    ani_method getMethod;
    GetMethodData(&object, &voidMethod, &getMethod);
    ani_int a = 4;
    ani_int b = 5;

    ani_int sum;
    ASSERT_EQ(env_->Object_CallMethod_Void(object, voidMethod, a, b), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, getMethod, &sum), ANI_OK);
    ASSERT_EQ(sum, a + b);
}

TEST_F(CallObjectMethodVoidTest, object_call_method_void)
{
    ani_object object;
    ani_method voidMethod;
    ani_method getMethod;
    GetMethodData(&object, &voidMethod, &getMethod);
    ani_int a = 6;
    ani_int b = 7;

    ani_int sum;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Void(env_, object, voidMethod, a, b), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, getMethod, &sum), ANI_OK);
    ASSERT_EQ(sum, a + b);
}

TEST_F(CallObjectMethodVoidTest, call_method_void_v_invalid_method)
{
    ani_object object;
    ani_method voidMethod;
    ani_method getMethod;
    GetMethodData(&object, &voidMethod, &getMethod);
    ani_int a = 6;
    ani_int b = 7;

    ASSERT_EQ(env_->Object_CallMethod_Void(object, nullptr, a, b), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodVoidTest, call_method_void_v_invalid_object)
{
    ani_object object;
    ani_method voidMethod;
    ani_method getMethod;
    GetMethodData(&object, &voidMethod, &getMethod);
    ani_int a = 6;
    ani_int b = 7;

    ASSERT_EQ(env_->Object_CallMethod_Void(nullptr, voidMethod, a, b), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodVoidTest, call_method_void_a_invalid_method)
{
    ani_object object;
    ani_method voidMethod;
    ani_method getMethod;
    GetMethodData(&object, &voidMethod, &getMethod);
    ani_value args[2];
    ani_int arg1 = 2;
    ani_int arg2 = 3;
    args[0].i = arg1;
    args[1].i = arg2;

    ASSERT_EQ(env_->Object_CallMethod_Void_A(object, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodVoidTest, call_method_void_a_invalid_object)
{
    ani_object object;
    ani_method voidMethod;
    ani_method getMethod;
    GetMethodData(&object, &voidMethod, &getMethod);
    ani_value args[2];
    ani_int arg1 = 2;
    ani_int arg2 = 3;
    args[0].i = arg1;
    args[1].i = arg2;

    ASSERT_EQ(env_->Object_CallMethod_Void_A(nullptr, voidMethod, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodVoidTest, call_method_void_a_invalid_args)
{
    ani_object object;
    ani_method voidMethod;
    ani_method getMethod;
    GetMethodData(&object, &voidMethod, &getMethod);

    ASSERT_EQ(env_->Object_CallMethod_Void_A(object, voidMethod, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
