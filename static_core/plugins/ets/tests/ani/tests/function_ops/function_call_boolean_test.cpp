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

class FunctionCallBooleanTest : public AniTest {
public:
    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("Lops;", &ns), ANI_OK);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "or", "ZZ:Z", &fn), ANI_OK);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallBooleanTest, function_call_boolean)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Function_Call_Boolean(env_, fn, &result, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_boolean result;
    ASSERT_EQ(env_->Function_Call_Boolean(fn, &result, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean result;
    ASSERT_EQ(env_->Function_Call_Boolean_A(fn, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_invalid_function)
{
    ani_boolean result;
    ASSERT_EQ(env_->c_api->Function_Call_Boolean(env_, nullptr, &result, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallBooleanTest, function_call_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_EQ(env_->c_api->Function_Call_Boolean(env_, fn, nullptr, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_a_invalid_function)
{
    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean result;
    ASSERT_EQ(env_->Function_Call_Boolean_A(nullptr, &result, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ASSERT_EQ(env_->Function_Call_Boolean_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_boolean result;
    ASSERT_EQ(env_->Function_Call_Boolean_A(fn, &result, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
