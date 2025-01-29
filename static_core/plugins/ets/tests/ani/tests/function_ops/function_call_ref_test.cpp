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

class FunctionCallRefTest : public AniTest {
public:
    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("Lops;", &ns), ANI_OK);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "concat", "II:Lstd/core/String;", &fn), ANI_OK);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallRefTest, function_call_ref)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_ref ref {};
    ASSERT_EQ(env_->c_api->Function_Call_Ref(env_, fn, &ref, 1, 0), ANI_OK);

    std::string result;
    GetStdString(static_cast<ani_string>(ref), result);
    ASSERT_STREQ(result.c_str(), "10");
}

TEST_F(FunctionCallRefTest, function_call_ref_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0].i = 2U;
    args[1].i = 1U;
    ani_ref ref {};
    ASSERT_EQ(env_->Function_Call_Ref_A(fn, &ref, args), ANI_OK);

    std::string result;
    GetStdString(static_cast<ani_string>(ref), result);
    ASSERT_STREQ(result.c_str(), "21");
}

TEST_F(FunctionCallRefTest, function_call_ref_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_ref ref {};
    ASSERT_EQ(env_->Function_Call_Ref(fn, &ref, 1U, 2U), ANI_OK);

    std::string result;
    GetStdString(static_cast<ani_string>(ref), result);
    ASSERT_STREQ(result.c_str(), "12");
}

TEST_F(FunctionCallRefTest, function_call_ref_invalid_function)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_ref ref {};
    ASSERT_EQ(env_->c_api->Function_Call_Ref(env_, nullptr, &ref, 1, 0), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, function_call_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_EQ(env_->c_api->Function_Call_Ref(env_, fn, nullptr, 1, 0), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, function_call_ref_a_invalid_function)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0].i = 2U;
    args[1].i = 1U;
    ani_ref ref {};
    ASSERT_EQ(env_->Function_Call_Ref_A(nullptr, &ref, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, function_call_ref_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0].i = 2U;
    args[1].i = 1U;
    ASSERT_EQ(env_->Function_Call_Ref_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, function_call_ref_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_ref ref {};
    ASSERT_EQ(env_->Function_Call_Ref_A(fn, &ref, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, function_call_ref_v_invalid_function)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_ref ref {};
    ASSERT_EQ(env_->Function_Call_Ref(nullptr, &ref, 1U, 2U), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, function_call_ref_v_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_EQ(env_->Function_Call_Ref(fn, nullptr, 1U, 2U), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
