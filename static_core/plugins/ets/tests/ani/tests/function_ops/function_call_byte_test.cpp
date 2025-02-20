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

class FunctionCallByteTest : public AniTest {
public:
    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("Lops;", &ns), ANI_OK);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "sum", "BB:B", &fn), ANI_OK);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallByteTest, function_call_byte)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ani_byte argOne = 5U;  // 5 : value of args
    ani_byte argTwo = 6U;  // 6 : value of args
    ani_byte sum;
    ASSERT_EQ(env_->c_api->Function_Call_Byte(env_, fn, &sum, argOne, argTwo), ANI_OK);
    ASSERT_EQ(sum, argOne + argTwo);
}

TEST_F(FunctionCallByteTest, function_call_byte_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ani_byte argOne = 5U;  // 5 : value of args
    ani_byte argTwo = 6U;  // 6 : value of args
    ani_byte sum;
    ASSERT_EQ(env_->Function_Call_Byte(fn, &sum, argOne, argTwo), ANI_OK);
    ASSERT_EQ(sum, argOne + argTwo);
}

TEST_F(FunctionCallByteTest, function_call_byte_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].b = 5U;  // 5 : value of args
    args[1U].b = 6U;  // 6 : value of args

    ani_byte sum;
    ASSERT_EQ(env_->Function_Call_Byte_A(fn, &sum, args), ANI_OK);
    ASSERT_EQ(sum, args[0U].b + args[1U].b);
}

TEST_F(FunctionCallByteTest, function_call_byte_invalid_function)
{
    ani_byte argOne = 5U;  // 5 : value of args
    ani_byte argTwo = 6U;  // 6 : value of args
    ani_byte sum;
    ASSERT_EQ(env_->Function_Call_Byte(nullptr, &sum, argOne, argTwo), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallByteTest, function_call_byte_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ani_byte argOne = 5U;  // 5 : value of args
    ani_byte argTwo = 6U;  // 6 : value of args
    ASSERT_EQ(env_->Function_Call_Byte(fn, nullptr, argOne, argTwo), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallByteTest, function_call_byte_a_invalid_function)
{
    ani_value args[2U];
    args[0U].b = 5U;  // 5 : value of args
    args[1U].b = 6U;  // 6 : value of args

    ani_byte sum;
    ASSERT_EQ(env_->Function_Call_Byte_A(nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallByteTest, function_call_byte_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].b = 5U;  // 5 : value of args
    args[1U].b = 6U;  // 6 : value of args

    ASSERT_EQ(env_->Function_Call_Byte_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallByteTest, function_call_byte_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_byte sum;
    ASSERT_EQ(env_->Function_Call_Byte_A(fn, &sum, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
