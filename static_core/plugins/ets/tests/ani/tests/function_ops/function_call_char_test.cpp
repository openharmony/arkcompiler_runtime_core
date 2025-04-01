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

class FunctionCallCharTest : public AniTest {
public:
    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("Lfunction_call_char_test/ops;", &ns), ANI_OK);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "sub", "CC:C", &fn), ANI_OK);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallCharTest, function_call_char)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ani_char argOne = 'C';
    ani_char argTwo = 'A';
    ani_char sub;
    ASSERT_EQ(env_->c_api->Function_Call_Char(env_, fn, &sub, argOne, argTwo), ANI_OK);
    ASSERT_EQ(sub, argOne - argTwo);
}

TEST_F(FunctionCallCharTest, function_call_char_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ani_char argOne = 'C';
    ani_char argTwo = 'A';
    ani_char sub;
    ASSERT_EQ(env_->Function_Call_Char(fn, &sub, argOne, argTwo), ANI_OK);
    ASSERT_EQ(sub, argOne - argTwo);
}

TEST_F(FunctionCallCharTest, function_call_char_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].c = 'C';
    args[1U].c = 'A';

    ani_char sub;
    ASSERT_EQ(env_->Function_Call_Char_A(fn, &sub, args), ANI_OK);
    ASSERT_EQ(sub, args[0U].b - args[1U].b);
}

TEST_F(FunctionCallCharTest, function_call_char_invalid_function)
{
    ani_char argOne = 'C';
    ani_char argTwo = 'A';
    ani_char sub;
    ASSERT_EQ(env_->c_api->Function_Call_Char(env_, nullptr, &sub, argOne, argTwo), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, function_call_char_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ani_char argOne = 'C';
    ani_char argTwo = 'A';
    ASSERT_EQ(env_->c_api->Function_Call_Char(env_, fn, nullptr, argOne, argTwo), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, function_call_char_v_invalid_function)
{
    ani_char argOne = 'C';
    ani_char argTwo = 'A';
    ani_char sub;
    ASSERT_EQ(env_->Function_Call_Char(nullptr, &sub, argOne, argTwo), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, function_call_char_v_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ani_char argOne = 'C';
    ani_char argTwo = 'A';
    ASSERT_EQ(env_->Function_Call_Char(fn, nullptr, argOne, argTwo), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, function_call_char_a_invalid_function)
{
    ani_value args[2U];
    args[0U].c = 'A';
    args[1U].c = 'C';

    ani_char sub;
    ASSERT_EQ(env_->Function_Call_Char_A(nullptr, &sub, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, function_call_char_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].c = 'A';
    args[1U].c = 'C';

    ASSERT_EQ(env_->Function_Call_Char_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, function_call_char_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_char sub;
    ASSERT_EQ(env_->Function_Call_Char_A(fn, &sub, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)