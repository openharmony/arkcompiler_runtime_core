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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class CallStaticMethodTest : public AniTest {
public:
    static constexpr ani_long VAL1 = 123;
    static constexpr ani_long VAL2 = 456;

    void GetMethodData(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LOperations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_GetStaticMethod(cls, "sum", "JJ:J", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }
};

TEST_F(CallStaticMethodTest, call_static_method_long)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_long sum;
    const ani_long v1 = VAL1;
    const ani_long v2 = VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Long(env_, cls, method, &sum, v1, v2), ANI_OK);
    ASSERT_EQ(sum, v1 + v2);
}

TEST_F(CallStaticMethodTest, call_static_method_long_v)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_long sum;
    const ani_long v1 = VAL1;
    const ani_long v2 = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(cls, method, &sum, v1, v2), ANI_OK);
    ASSERT_EQ(sum, v1 + v2);
}

TEST_F(CallStaticMethodTest, call_static_method_long_A)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_long_null_class)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_long sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(nullptr, method, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_null_method)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_long sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(cls, nullptr, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_null_result)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ASSERT_EQ(env_->Class_CallStaticMethod_Long(cls, method, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_A_null_class)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(nullptr, method, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_A_null_method)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_A_null_result)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, method, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_A_null_args)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_long sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, method, &sum, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
