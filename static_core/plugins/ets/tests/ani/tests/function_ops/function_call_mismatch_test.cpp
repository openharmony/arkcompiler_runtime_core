/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <vector>
#include <string>
#include <functional>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

struct FunctionCallMismatchParam {
    std::string testName;
    std::string functionName;
    std::string signature;
    ani_status expectedStatus;

    std::function<ani_status(ani_env *, ani_function)> callV;
    std::function<ani_status(ani_env *, ani_function)> callA;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class FunctionCallReturnTypeTest : public AniTest {
public:
    void GetFunction(const char *name, const char *sig, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_mismatch_test.TypeProvider", &ns), ANI_OK);
        ASSERT_NE(ns, nullptr);

        ASSERT_EQ(env_->Namespace_FindFunction(ns, name, sig, fnResult), ANI_OK);
        ASSERT_NE(*fnResult, nullptr);
    }

protected:
    void RunTest(const FunctionCallMismatchParam &param)
    {
        ani_function fn {};
        GetFunction(param.functionName.c_str(), param.signature.c_str(), &fn);

        ani_status statusV = param.callV(env_, fn);
        EXPECT_EQ(statusV, param.expectedStatus) << "Failed Function_Call: " << param.testName;

        ani_status statusA = param.callA(env_, fn);
        EXPECT_EQ(statusA, param.expectedStatus) << "Failed Function_Call_A: " << param.testName;
    }
};

template <typename T, typename FuncV, typename FuncA>
FunctionCallMismatchParam CreateFunctionCallMismatchParam(const std::string &etsMethodName, const std::string &sig,
                                                          ani_status expectedStatus, FuncV callV, FuncA callA)
{
    auto callVLambda = [callV](ani_env *env, ani_function f) -> ani_status {
        T res;
        return (env->*callV)(f, &res);
    };

    auto callALambda = [callA](ani_env *env, ani_function f) -> ani_status {
        T res;
        ani_value args[1] {};
        return (env->*callA)(f, &res, args);
    };

    return FunctionCallMismatchParam {"Call" + etsMethodName, etsMethodName,          sig,
                                      expectedStatus,         std::move(callVLambda), std::move(callALambda)};
}

FunctionCallMismatchParam CreateFunctionCallMismatchVoidParam(const std::string &etsMethodName, const std::string &sig,
                                                              ani_status status)
{
    auto callVoid = [](ani_env *env, ani_function f) -> ani_status { return env->Function_Call_Void(f); };
    auto callVoidA = [](ani_env *env, ani_function f) -> ani_status {
        ani_value args[1] {};
        return env->Function_Call_Void_A(f, args);
    };

    return FunctionCallMismatchParam {"VoidCall" + etsMethodName, etsMethodName,       sig, status,
                                      std::move(callVoid),        std::move(callVoidA)};
}

static std::vector<FunctionCallMismatchParam> GetFunctionCallSuccessParams()
{
    return {CreateFunctionCallMismatchParam<ani_boolean>("retBool", ":z", ANI_OK, &ani_env::Function_Call_Boolean,
                                                         &ani_env::Function_Call_Boolean_A),
            CreateFunctionCallMismatchParam<ani_byte>("retByte", ":b", ANI_OK, &ani_env::Function_Call_Byte,
                                                      &ani_env::Function_Call_Byte_A),
            CreateFunctionCallMismatchParam<ani_char>("retChar", ":c", ANI_OK, &ani_env::Function_Call_Char,
                                                      &ani_env::Function_Call_Char_A),
            CreateFunctionCallMismatchParam<ani_short>("retShort", ":s", ANI_OK, &ani_env::Function_Call_Short,
                                                       &ani_env::Function_Call_Short_A),
            CreateFunctionCallMismatchParam<ani_int>("retInt", ":i", ANI_OK, &ani_env::Function_Call_Int,
                                                     &ani_env::Function_Call_Int_A),
            CreateFunctionCallMismatchParam<ani_long>("retLong", ":l", ANI_OK, &ani_env::Function_Call_Long,
                                                      &ani_env::Function_Call_Long_A),
            CreateFunctionCallMismatchParam<ani_float>("retFloat", ":f", ANI_OK, &ani_env::Function_Call_Float,
                                                       &ani_env::Function_Call_Float_A),
            CreateFunctionCallMismatchParam<ani_double>("retDouble", ":d", ANI_OK, &ani_env::Function_Call_Double,
                                                        &ani_env::Function_Call_Double_A),
            CreateFunctionCallMismatchParam<ani_ref>("retRef", ":C{std.core.Array}", ANI_OK,
                                                     &ani_env::Function_Call_Ref, &ani_env::Function_Call_Ref_A),
            CreateFunctionCallMismatchVoidParam("retVoid", ":", ANI_OK)};
}

static std::vector<FunctionCallMismatchParam> GetFunctionCallTypeMismatchParams()
{
    return {CreateFunctionCallMismatchParam<ani_int>("retBool", ":z", ANI_INVALID_TYPE, &ani_env::Function_Call_Int,
                                                     &ani_env::Function_Call_Int_A),
            CreateFunctionCallMismatchParam<ani_boolean>(
                "retInt", ":i", ANI_INVALID_TYPE, &ani_env::Function_Call_Boolean, &ani_env::Function_Call_Boolean_A),
            CreateFunctionCallMismatchParam<ani_char>("retByte", ":b", ANI_INVALID_TYPE, &ani_env::Function_Call_Char,
                                                      &ani_env::Function_Call_Char_A),
            CreateFunctionCallMismatchParam<ani_long>("retDouble", ":d", ANI_INVALID_TYPE, &ani_env::Function_Call_Long,
                                                      &ani_env::Function_Call_Long_A),
            CreateFunctionCallMismatchParam<ani_ref>("retLong", ":l", ANI_INVALID_TYPE, &ani_env::Function_Call_Ref,
                                                     &ani_env::Function_Call_Ref_A)};
}

static std::vector<FunctionCallMismatchParam> GetFunctionCallVoidMismatchParams()
{
    return {CreateFunctionCallMismatchVoidParam("retInt", ":i", ANI_INVALID_TYPE),
            CreateFunctionCallMismatchVoidParam("retChar", ":c", ANI_INVALID_TYPE),
            CreateFunctionCallMismatchVoidParam("retRef", ":C{std.core.Array}", ANI_INVALID_TYPE),
            CreateFunctionCallMismatchParam<ani_int>("retVoid", ":", ANI_INVALID_TYPE, &ani_env::Function_Call_Int,
                                                     &ani_env::Function_Call_Int_A),
            CreateFunctionCallMismatchParam<ani_boolean>(
                "retVoid", ":", ANI_INVALID_TYPE, &ani_env::Function_Call_Boolean, &ani_env::Function_Call_Boolean_A),
            CreateFunctionCallMismatchParam<ani_ref>("retVoid", ":", ANI_INVALID_TYPE, &ani_env::Function_Call_Ref,
                                                     &ani_env::Function_Call_Ref_A)};
}

TEST_F(FunctionCallReturnTypeTest, Success)
{
    for (const auto &param : GetFunctionCallSuccessParams()) {
        RunTest(param);
    }
}

TEST_F(FunctionCallReturnTypeTest, TypeMismatch)
{
    for (const auto &param : GetFunctionCallTypeMismatchParams()) {
        RunTest(param);
    }
}

TEST_F(FunctionCallReturnTypeTest, VoidMismatch)
{
    for (const auto &param : GetFunctionCallVoidMismatchParams()) {
        RunTest(param);
    }
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)