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

class FunctionCallByteTest : public AniTest {
public:
    static constexpr ani_byte BYTE_VAL1 = 1U;
    static constexpr ani_byte BYTE_VAL2 = 2U;
    static constexpr ani_byte BYTE_VAL3 = 3U;

    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_module module {};
        ASSERT_EQ(env_->FindModule("L@functionModule/function_call_byte_test;", &module), ANI_OK);
        ASSERT_NE(module, nullptr);

        ani_namespace ns {};
        ASSERT_EQ(env_->Module_FindNamespace(module, "Lops;", &ns), ANI_OK);
        ASSERT_NE(ns, nullptr);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "sum", "BB:B", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallByteTest, function_call_byte)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ani_byte sum {};
    ASSERT_EQ(env_->c_api->Function_Call_Byte(env_, fn, &sum, BYTE_VAL1, BYTE_VAL2), ANI_OK);
    ASSERT_EQ(sum, BYTE_VAL1 + BYTE_VAL2);
}

TEST_F(FunctionCallByteTest, function_call_byte_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ani_byte sum {};
    ASSERT_EQ(env_->Function_Call_Byte(fn, &sum, BYTE_VAL1, BYTE_VAL2), ANI_OK);
    ASSERT_EQ(sum, BYTE_VAL1 + BYTE_VAL2);
}

TEST_F(FunctionCallByteTest, function_call_byte_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].b = BYTE_VAL1;
    args[1U].b = BYTE_VAL2;

    ani_byte sum {};
    ASSERT_EQ(env_->Function_Call_Byte_A(fn, &sum, args), ANI_OK);
    ASSERT_EQ(sum, BYTE_VAL1 + BYTE_VAL2);
}

TEST_F(FunctionCallByteTest, function_call_byte_invalid_function)
{
    ani_byte sum {};
    ASSERT_EQ(env_->Function_Call_Byte(nullptr, &sum, BYTE_VAL1, BYTE_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallByteTest, function_call_byte_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ASSERT_EQ(env_->Function_Call_Byte(fn, nullptr, BYTE_VAL1, BYTE_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallByteTest, function_call_byte_a_invalid_function)
{
    ani_value args[2U];
    args[0U].b = BYTE_VAL1;
    args[1U].b = BYTE_VAL2;

    ani_byte sum {};
    ASSERT_EQ(env_->Function_Call_Byte_A(nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallByteTest, function_call_byte_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].b = BYTE_VAL1;
    args[1U].b = BYTE_VAL2;

    ASSERT_EQ(env_->Function_Call_Byte_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallByteTest, function_call_byte_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_byte sum {};
    ASSERT_EQ(env_->Function_Call_Byte_A(fn, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallByteTest, function_call_byte_001)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@functionModule/function_call_byte_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "LA;", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "byteFunctionA", "BB:B", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_byte value {};
    ASSERT_EQ(env_->Function_Call_Byte(fA, &value, BYTE_VAL1, BYTE_VAL2), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1 + BYTE_VAL2);

    ani_value args[2U];
    args[0U].b = BYTE_VAL1;
    args[1U].b = BYTE_VAL2;
    ASSERT_EQ(env_->Function_Call_Byte_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1 + BYTE_VAL2);
}

TEST_F(FunctionCallByteTest, function_call_byte_002)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@functionModule/function_call_byte_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_namespace nA {};
    ani_namespace nB {};
    ani_function fB {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "LA;", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Module_FindNamespace(module, "LA/B;", &nB), ANI_OK);
    ASSERT_NE(nB, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nB, "byteFunctionB", "BB:B", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_byte value {};
    ASSERT_EQ(env_->Function_Call_Byte(fB, &value, BYTE_VAL1, BYTE_VAL2), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1 + BYTE_VAL2);

    ani_value args[2U];
    args[0U].b = BYTE_VAL1;
    args[1U].b = BYTE_VAL2;
    ASSERT_EQ(env_->Function_Call_Byte_A(fB, &value, args), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1 + BYTE_VAL2);
}

TEST_F(FunctionCallByteTest, function_call_byte_003)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@functionModule/function_call_byte_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "LA;", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "byteFunctionA", "BBB:B", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_byte value {};
    ASSERT_EQ(env_->Function_Call_Byte(fA, &value, BYTE_VAL1, BYTE_VAL2, BYTE_VAL3), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1 + BYTE_VAL2 + BYTE_VAL3);

    ani_value args[3U];
    args[0U].b = BYTE_VAL1;
    args[1U].b = BYTE_VAL2;
    args[2U].b = BYTE_VAL3;
    ASSERT_EQ(env_->Function_Call_Byte_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1 + BYTE_VAL2 + BYTE_VAL3);
}

TEST_F(FunctionCallByteTest, function_call_byte_004)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@functionModule/function_call_byte_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fA {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "BB:B", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_byte value {};
    ASSERT_EQ(env_->Function_Call_Byte(fA, &value, BYTE_VAL1, BYTE_VAL2), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1 + BYTE_VAL2);

    ani_value args[2U];
    args[0U].b = BYTE_VAL1;
    args[1U].b = BYTE_VAL2;
    ASSERT_EQ(env_->Function_Call_Byte_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1 + BYTE_VAL2);
}

TEST_F(FunctionCallByteTest, function_call_byte_005)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@functionModule/function_call_byte_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_byte value {};
    ani_function fB {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "BBB:B", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_value argsB[3U];
    argsB[0U].b = BYTE_VAL1;
    argsB[1U].b = BYTE_VAL2;
    argsB[2U].b = BYTE_VAL3;
    ASSERT_EQ(env_->Function_Call_Byte(fB, &value, BYTE_VAL1, BYTE_VAL2, BYTE_VAL3), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1 + BYTE_VAL2 + BYTE_VAL3);
    ASSERT_EQ(env_->Function_Call_Byte_A(fB, &value, argsB), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1 + BYTE_VAL2 + BYTE_VAL3);
}

TEST_F(FunctionCallByteTest, function_call_byte_006)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@functionModule/function_call_byte_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "LA;", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "nestedFunction", "BB:B", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_byte value {};
    ASSERT_EQ(env_->Function_Call_Byte(fA, &value, BYTE_VAL1, BYTE_VAL2), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1 + BYTE_VAL2);

    ani_value args[2U];
    args[0U].b = BYTE_VAL1;
    args[1U].b = BYTE_VAL2;
    ASSERT_EQ(env_->Function_Call_Byte_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1 + BYTE_VAL2);
}

TEST_F(FunctionCallByteTest, function_call_byte_007)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@functionModule/function_call_byte_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "LA;", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "recursiveFunction", "I:B", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_byte value {};
    const ani_int value1 = 5;
    const ani_byte result = 15;
    ASSERT_EQ(env_->Function_Call_Byte(fA, &value, value1), ANI_OK);
    ASSERT_EQ(value, result);

    ani_value args[1U];
    args[0U].i = value1;
    ASSERT_EQ(env_->Function_Call_Byte_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, result);
}

TEST_F(FunctionCallByteTest, function_call_byte_008)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@functionModule/function_call_byte_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "LA;", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "calculateSum", "BBCI:B", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_byte value {};
    const ani_char cValue = 'A';
    const ani_int iValue = 1.0;
    ASSERT_EQ(env_->Function_Call_Byte(fA, &value, BYTE_VAL1, BYTE_VAL2, cValue, iValue), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL2 - BYTE_VAL1);

    const ani_char cValue1 = 'B';
    ani_value args[4U];
    args[0U].b = BYTE_VAL1;
    args[1U].b = BYTE_VAL2;
    args[2U].c = cValue1;
    args[3U].i = iValue;
    ASSERT_EQ(env_->Function_Call_Byte_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1);

    const ani_int iValue1 = 2.0;
    ASSERT_EQ(env_->Function_Call_Byte(fA, &value, BYTE_VAL1, BYTE_VAL2, cValue1, iValue1), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL2 + BYTE_VAL1);
}

TEST_F(FunctionCallByteTest, function_call_byte_009)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@functionModule/function_call_byte_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "LA;", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "byteFunctionA", "BB:B", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_byte value {};
    ASSERT_EQ(env_->Function_Call_Byte(fA, &value, BYTE_VAL1, BYTE_VAL2), ANI_OK);
    ASSERT_EQ(value, BYTE_VAL1 + BYTE_VAL2);

    const ani_byte value1 = 5U;
    ASSERT_EQ(env_->Function_Call_Byte(fA, &value, value1, BYTE_VAL2), ANI_OK);
    ASSERT_EQ(value, value1 + BYTE_VAL2);

    const ani_byte value2 = 2U;
    ani_value args[2U];
    args[0U].b = value1;
    args[1U].b = value2;
    ASSERT_EQ(env_->Function_Call_Byte_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, value1 + value2);

    const ani_byte value3 = 4U;
    ASSERT_EQ(env_->Function_Call_Byte(fA, &value, value2, value3), ANI_OK);
    ASSERT_EQ(value, value2 + value3);

    const ani_byte value4 = 6U;
    ASSERT_EQ(env_->Function_Call_Byte(fA, &value, value3, value4), ANI_OK);
    ASSERT_EQ(value, value3 + value4);
}

TEST_F(FunctionCallByteTest, function_call_byte_010)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].b = BYTE_VAL1;
    args[1U].b = BYTE_VAL2;

    ani_byte result = 0;
    ASSERT_EQ(env_->c_api->Function_Call_Byte(nullptr, fn, &result, BYTE_VAL1, BYTE_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Function_Call_Byte_A(nullptr, fn, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Byte(nullptr, &result, BYTE_VAL1, BYTE_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Byte_A(nullptr, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Byte(fn, nullptr, BYTE_VAL1, BYTE_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Byte_A(fn, nullptr, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Byte(fn, &result, nullptr), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Byte_A(fn, &result, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
