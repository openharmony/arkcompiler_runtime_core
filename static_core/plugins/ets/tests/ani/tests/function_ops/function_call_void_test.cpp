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

class FunctionCallVoidTest : public AniTest {
public:
    void GetMethod(ani_namespace *nsResult, ani_function *fnResult1, ani_function *fnResult2)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("Lfunction_call_void_test/ops;", &ns), ANI_OK);

        ani_function fn1 {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "sum", "II:V", &fn1), ANI_OK);

        ani_function fn2 {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "getCount", ":I", &fn2), ANI_OK);

        *nsResult = ns;
        *fnResult1 = fn1;
        *fnResult2 = fn2;
    }
};

TEST_F(FunctionCallVoidTest, function_call_void)
{
    ani_namespace ns {};
    ani_function fn1 {};
    ani_function fn2 {};
    GetMethod(&ns, &fn1, &fn2);

    ani_int arg1 = 5U;  // 5 : value of args
    ani_int arg2 = 6U;  // 6 : value of args

    ani_int sum;
    ASSERT_EQ(env_->c_api->Function_Call_Void(env_, fn1, arg1, arg2), ANI_OK);
    ASSERT_EQ(env_->c_api->Function_Call_Int(env_, fn2, &sum, nullptr), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(FunctionCallVoidTest, function_call_void_v)
{
    ani_namespace ns {};
    ani_function fn1 {};
    ani_function fn2 {};
    GetMethod(&ns, &fn1, &fn2);

    ani_int arg1 = 5U;  // 5 : value of args
    ani_int arg2 = 6U;  // 6 : value of args

    ani_int sum;
    ASSERT_EQ(env_->Function_Call_Void(fn1, arg1, arg2), ANI_OK);
    ASSERT_EQ(env_->c_api->Function_Call_Int(env_, fn2, &sum, nullptr), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(FunctionCallVoidTest, function_call_void_a)
{
    ani_namespace ns {};
    ani_function fn1 {};
    ani_function fn2 {};
    GetMethod(&ns, &fn1, &fn2);

    ani_value args[2U];
    args[0U].i = 5U;  // 5 : value of args
    args[1U].i = 6U;  // 6 : value of args

    ani_int sum;
    ASSERT_EQ(env_->c_api->Function_Call_Void_A(env_, fn1, args), ANI_OK);
    ASSERT_EQ(env_->c_api->Function_Call_Int(env_, fn2, &sum, nullptr), ANI_OK);
    ASSERT_EQ(sum, args[0U].i + args[1U].i);
}

TEST_F(FunctionCallVoidTest, function_call_void_invalid_function)
{
    ani_int arg1 = 5U;  // 5 : value of args
    ani_int arg2 = 6U;  // 6 : value of args

    ASSERT_EQ(env_->Function_Call_Void(nullptr, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallVoidTest, function_call_void_a_invalid_function)
{
    ani_value args[2U];
    args[0U].i = 5U;  // 5 : value of args
    args[1U].i = 6U;  // 6 : value of args

    ASSERT_EQ(env_->Function_Call_Void_A(nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallVoidTest, function_call_void_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn1 {};
    ani_function fn2 {};
    GetMethod(&ns, &fn1, &fn2);

    ASSERT_EQ(env_->Function_Call_Void_A(fn1, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)