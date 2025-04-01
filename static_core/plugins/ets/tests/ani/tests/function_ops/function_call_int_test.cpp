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

class FunctionCallTest : public AniTest {
public:
    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("Lfunction_call_int_test/ops;", &ns), ANI_OK);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "sum", "II:I", &fn), ANI_OK);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallTest, function_call_int)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_int value;
    int argOne = 5U;  // 5 : value of args
    int argTwo = 6U;  // 6 : value of args
    ASSERT_EQ(env_->c_api->Function_Call_Int(env_, fn, &value, argOne, argTwo), ANI_OK);
    ASSERT_EQ(value, argOne + argTwo);
}

TEST_F(FunctionCallTest, function_call_int_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].i = 5U;  // 5 : value of args
    args[1U].i = 6U;  // 6 : value of args
    ani_int value;
    ASSERT_EQ(env_->Function_Call_Int_A(fn, &value, args), ANI_OK);
    ASSERT_EQ(value, args[0U].i + args[1U].i);
}

TEST_F(FunctionCallTest, function_call_int_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_int value;
    int argOne = 5U;  // 5 : value of args
    int argTwo = 6U;  // 6 : value of args
    ASSERT_EQ(env_->Function_Call_Int(fn, &value, argOne, argTwo), ANI_OK);
    ASSERT_EQ(value, argOne + argTwo);
}

TEST_F(FunctionCallTest, function_call_int_invalid_function)
{
    ani_int value;
    int argOne = 5U;  // 5 : value of args
    int argTwo = 6U;  // 6 : value of args
    ASSERT_EQ(env_->c_api->Function_Call_Int(env_, nullptr, &value, argOne, argTwo), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    int argOne = 5U;  // 5 : value of args
    int argTwo = 6U;  // 6 : value of args
    ASSERT_EQ(env_->c_api->Function_Call_Int(env_, fn, nullptr, argOne, argTwo), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_int_a_invalid_function)
{
    ani_value args[2U];
    args[0U].i = 5U;  // 5 : value of args
    args[1U].i = 6U;  // 6 : value of args
    ani_int value;
    ASSERT_EQ(env_->Function_Call_Int_A(nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_int_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].i = 5U;  // 5 : value of args
    args[1U].i = 6U;  // 6 : value of args
    ASSERT_EQ(env_->Function_Call_Int_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_int_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_int value;
    ASSERT_EQ(env_->Function_Call_Int_A(fn, &value, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_int_v_invalid_function)
{
    ani_int value;
    int argOne = 5U;  // 5 : value of args
    int argTwo = 6U;  // 6 : value of args
    ASSERT_EQ(env_->Function_Call_Int(nullptr, &value, argOne, argTwo), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_int_v_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    int argOne = 5U;  // 5 : value of args
    int argTwo = 6U;  // 6 : value of args
    ASSERT_EQ(env_->Function_Call_Int(fn, nullptr, argOne, argTwo), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)