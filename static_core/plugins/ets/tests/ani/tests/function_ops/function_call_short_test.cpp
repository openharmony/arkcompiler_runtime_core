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
        ASSERT_EQ(env_->FindNamespace("Lfunction_call_short_test/ops;", &ns), ANI_OK);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "sum", "SS:S", &fn), ANI_OK);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallTest, function_call_short)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_short value;
    int16_t argOne = 5U;  // 5 : value of args
    int16_t argTwo = 6U;  // 6 : value of args
    ASSERT_EQ(env_->c_api->Function_Call_Short(env_, fn, &value, argOne, argTwo), ANI_OK);
    ASSERT_EQ(value, argOne + argTwo);
}

TEST_F(FunctionCallTest, function_call_short_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].s = 5U;  // 5 : value of args
    args[1U].s = 6U;  // 6 : value of args
    ani_short value;
    ASSERT_EQ(env_->Function_Call_Short_A(fn, &value, args), ANI_OK);
    ASSERT_EQ(value, args[0U].s + args[1U].s);
}

TEST_F(FunctionCallTest, function_call_short_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_short value;
    int16_t argOne = 5U;  // 5 : value of args
    int16_t argTwo = 6U;  // 6 : value of args
    ASSERT_EQ(env_->Function_Call_Short(fn, &value, argOne, argTwo), ANI_OK);
    ASSERT_EQ(value, argOne + argTwo);
}

TEST_F(FunctionCallTest, function_call_short_invalid_function)
{
    ani_short value;
    int16_t argOne = 5U;  // 5 : value of args
    int16_t argTwo = 6U;  // 6 : value of args
    ASSERT_EQ(env_->c_api->Function_Call_Short(env_, nullptr, &value, argOne, argTwo), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    int16_t argOne = 5U;  // 5 : value of args
    int16_t argTwo = 6U;  // 6 : value of args
    ASSERT_EQ(env_->c_api->Function_Call_Short(env_, fn, nullptr, argOne, argTwo), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_short_a_invalid_function)
{
    ani_value args[2U];
    args[0U].s = 5U;  // 5 : value of args
    args[1U].s = 6U;  // 6 : value of args
    ani_short value;
    ASSERT_EQ(env_->Function_Call_Short_A(nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_short_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].s = 5U;  // 5 : value of args
    args[1U].s = 6U;  // 6 : value of args
    ASSERT_EQ(env_->Function_Call_Short_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_short_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_short value;
    ASSERT_EQ(env_->Function_Call_Short_A(fn, &value, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_short_v_invalid_function)
{
    ani_short value;
    int16_t argOne = 5U;  // 5 : value of args
    int16_t argTwo = 6U;  // 6 : value of args
    ASSERT_EQ(env_->Function_Call_Short(nullptr, &value, argOne, argTwo), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_short_v_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    int16_t argOne = 5U;  // 5 : value of args
    int16_t argTwo = 6U;  // 6 : value of args
    ASSERT_EQ(env_->Function_Call_Short(fn, nullptr, argOne, argTwo), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)