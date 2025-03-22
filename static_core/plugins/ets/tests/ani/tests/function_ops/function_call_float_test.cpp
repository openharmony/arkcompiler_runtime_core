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

class FunctionCallFloatTest : public AniTest {
public:
    static constexpr ani_float FLOAT_VAL1 = 1.5F;
    static constexpr ani_float FLOAT_VAL2 = 2.5F;
    static constexpr size_t ARG_COUNT = 2U;

    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("Lfunction_call_float_test/ops;", &ns), ANI_OK);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "sum", "FF:F", &fn), ANI_OK);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallFloatTest, function_call_float)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_float value;
    ASSERT_EQ(env_->c_api->Function_Call_Float(env_, fn, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(FunctionCallFloatTest, function_call_float_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float value;
    ASSERT_EQ(env_->Function_Call_Float_A(fn, &value, args), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(FunctionCallFloatTest, function_call_float_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_float value;
    ASSERT_EQ(env_->Function_Call_Float(fn, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(FunctionCallFloatTest, function_call_float_invalid_function)
{
    ani_float value;
    ASSERT_EQ(env_->c_api->Function_Call_Float(env_, nullptr, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, function_call_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ASSERT_EQ(env_->c_api->Function_Call_Float(env_, fn, nullptr, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, function_call_float_a_invalid_function)
{
    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float value;
    ASSERT_EQ(env_->Function_Call_Float_A(nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, function_call_float_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ASSERT_EQ(env_->Function_Call_Float_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, function_call_float_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_float value;
    ASSERT_EQ(env_->Function_Call_Float_A(fn, &value, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, function_call_float_v_invalid_function)
{
    ani_float value;
    ASSERT_EQ(env_->Function_Call_Float(nullptr, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, function_call_float_v_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ASSERT_EQ(env_->Function_Call_Float(fn, nullptr, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
