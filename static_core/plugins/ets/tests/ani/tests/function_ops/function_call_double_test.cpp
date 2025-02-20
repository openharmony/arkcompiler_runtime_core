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

class FunctionCallDoubleTest : public AniTest {
public:
    static constexpr ani_double VAL1 = 1.5;
    static constexpr ani_double VAL2 = 2.5;
    static constexpr size_t ARG_COUNT = 2U;

    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("Lops;", &ns), ANI_OK);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "sum", "DD:D", &fn), ANI_OK);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallDoubleTest, function_call_double)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_double value;
    ASSERT_EQ(env_->c_api->Function_Call_Double(env_, fn, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(FunctionCallDoubleTest, function_call_double_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double value;
    ASSERT_EQ(env_->Function_Call_Double_A(fn, &value, args), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(FunctionCallDoubleTest, function_call_double_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_double value;
    ASSERT_EQ(env_->Function_Call_Double(fn, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(FunctionCallDoubleTest, function_call_double_invalid_function)
{
    ani_double value;
    ASSERT_EQ(env_->c_api->Function_Call_Double(env_, nullptr, &value, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallDoubleTest, function_call_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ASSERT_EQ(env_->c_api->Function_Call_Double(env_, fn, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallDoubleTest, function_call_double_a_invalid_function)
{
    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double value;
    ASSERT_EQ(env_->Function_Call_Double_A(nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallDoubleTest, function_call_double_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ASSERT_EQ(env_->Function_Call_Double_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallDoubleTest, function_call_double_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_double value;
    ASSERT_EQ(env_->Function_Call_Double_A(fn, &value, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallDoubleTest, function_call_double_v_invalid_function)
{
    ani_double value;
    ASSERT_EQ(env_->Function_Call_Double(nullptr, &value, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallDoubleTest, function_call_double_v_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ASSERT_EQ(env_->Function_Call_Double(fn, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
