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

struct CallStaticMismatchParam {
    std::string testName;
    std::string etsMethodName;
    std::string etsSignature;
    ani_status expectedStatus;

    std::function<ani_status(ani_env *, ani_class, ani_static_method)> callV;
    std::function<ani_status(ani_env *, ani_class, ani_static_method)> callA;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class CallStaticMethodReturnTypeTest : public AniTest {
protected:
    void RunTest(const CallStaticMismatchParam &param)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("call_static_method_mismatch_test.TypeProvider", &cls), ANI_OK);

        ani_static_method method {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, param.etsMethodName.c_str(), param.etsSignature.c_str(), &method),
                  ANI_OK);

        ani_status statusV = param.callV(env_, cls, method);
        EXPECT_EQ(statusV, param.expectedStatus) << "Failed _V call: " << param.testName;

        ani_status statusA = param.callA(env_, cls, method);
        EXPECT_EQ(statusA, param.expectedStatus) << "Failed _A call: " << param.testName;
    }
};

template <typename T, typename FuncV, typename FuncA>
CallStaticMismatchParam CreateCallStaticMismatchParam(const std::string &etsMethodName, const std::string &sig,
                                                      ani_status expectedStatus, FuncV callV, FuncA callA)
{
    auto callStaticV = [callV](ani_env *env, ani_class c, ani_static_method m) -> ani_status {
        T res;
        return (env->*callV)(c, m, &res);
    };
    auto callStaticA = [callA](ani_env *env, ani_class c, ani_static_method m) -> ani_status {
        T res;
        ani_value args[1] {};
        return (env->*callA)(c, m, &res, args);
    };

    return CallStaticMismatchParam {"Call" + etsMethodName, etsMethodName,          sig,
                                    expectedStatus,         std::move(callStaticV), std::move(callStaticA)};
}

CallStaticMismatchParam CreateCallStaticMismatchVoidParam(const std::string &etsMethodName, const std::string &sig,
                                                          ani_status status)
{
    auto callStaticVoid = [](ani_env *env, ani_class c, ani_static_method m) -> ani_status {
        return env->Class_CallStaticMethod_Void(c, m);
    };
    auto callStaticVoidA = [](ani_env *env, ani_class c, ani_static_method m) -> ani_status {
        ani_value args[1] {};
        return env->Class_CallStaticMethod_Void_A(c, m, args);
    };

    return CallStaticMismatchParam {"VoidCall" + etsMethodName, etsMethodName, sig, status, std::move(callStaticVoid),
                                    std::move(callStaticVoidA)};
}

static std::vector<CallStaticMismatchParam> GetSuccessParams()
{
    return {
        CreateCallStaticMismatchParam<ani_boolean>("retBool", ":z", ANI_OK, &ani_env::Class_CallStaticMethod_Boolean,
                                                   &ani_env::Class_CallStaticMethod_Boolean_A),
        CreateCallStaticMismatchParam<ani_byte>("retByte", ":b", ANI_OK, &ani_env::Class_CallStaticMethod_Byte,
                                                &ani_env::Class_CallStaticMethod_Byte_A),
        CreateCallStaticMismatchParam<ani_char>("retChar", ":c", ANI_OK, &ani_env::Class_CallStaticMethod_Char,
                                                &ani_env::Class_CallStaticMethod_Char_A),
        CreateCallStaticMismatchParam<ani_short>("retShort", ":s", ANI_OK, &ani_env::Class_CallStaticMethod_Short,
                                                 &ani_env::Class_CallStaticMethod_Short_A),
        CreateCallStaticMismatchParam<ani_int>("retInt", ":i", ANI_OK, &ani_env::Class_CallStaticMethod_Int,
                                               &ani_env::Class_CallStaticMethod_Int_A),
        CreateCallStaticMismatchParam<ani_long>("retLong", ":l", ANI_OK, &ani_env::Class_CallStaticMethod_Long,
                                                &ani_env::Class_CallStaticMethod_Long_A),
        CreateCallStaticMismatchParam<ani_float>("retFloat", ":f", ANI_OK, &ani_env::Class_CallStaticMethod_Float,
                                                 &ani_env::Class_CallStaticMethod_Float_A),
        CreateCallStaticMismatchParam<ani_double>("retDouble", ":d", ANI_OK, &ani_env::Class_CallStaticMethod_Double,
                                                  &ani_env::Class_CallStaticMethod_Double_A),
        CreateCallStaticMismatchParam<ani_ref>("retRef", ":C{std.core.Array}", ANI_OK,
                                               &ani_env::Class_CallStaticMethod_Ref,
                                               &ani_env::Class_CallStaticMethod_Ref_A),
        CreateCallStaticMismatchVoidParam("retVoid", ":", ANI_OK)};
}

static std::vector<CallStaticMismatchParam> GetPrimitiveMismatchParams()
{
    return {
        CreateCallStaticMismatchParam<ani_boolean>("retByte", ":b", ANI_INVALID_TYPE,
                                                   &ani_env::Class_CallStaticMethod_Boolean,
                                                   &ani_env::Class_CallStaticMethod_Boolean_A),
        CreateCallStaticMismatchParam<ani_boolean>("retInt", ":i", ANI_INVALID_TYPE,
                                                   &ani_env::Class_CallStaticMethod_Boolean,
                                                   &ani_env::Class_CallStaticMethod_Boolean_A),
        CreateCallStaticMismatchParam<ani_char>("retInt", ":i", ANI_INVALID_TYPE, &ani_env::Class_CallStaticMethod_Char,
                                                &ani_env::Class_CallStaticMethod_Char_A),
        CreateCallStaticMismatchParam<ani_byte>("retShort", ":s", ANI_INVALID_TYPE,
                                                &ani_env::Class_CallStaticMethod_Byte,
                                                &ani_env::Class_CallStaticMethod_Byte_A),
        CreateCallStaticMismatchParam<ani_short>("retInt", ":i", ANI_INVALID_TYPE,
                                                 &ani_env::Class_CallStaticMethod_Short,
                                                 &ani_env::Class_CallStaticMethod_Short_A),
        CreateCallStaticMismatchParam<ani_int>("retByte", ":b", ANI_INVALID_TYPE, &ani_env::Class_CallStaticMethod_Int,
                                               &ani_env::Class_CallStaticMethod_Int_A),
        CreateCallStaticMismatchParam<ani_int>("retLong", ":l", ANI_INVALID_TYPE, &ani_env::Class_CallStaticMethod_Int,
                                               &ani_env::Class_CallStaticMethod_Int_A),
        CreateCallStaticMismatchParam<ani_long>("retInt", ":i", ANI_INVALID_TYPE, &ani_env::Class_CallStaticMethod_Long,
                                                &ani_env::Class_CallStaticMethod_Long_A),
        CreateCallStaticMismatchParam<ani_float>("retDouble", ":d", ANI_INVALID_TYPE,
                                                 &ani_env::Class_CallStaticMethod_Float,
                                                 &ani_env::Class_CallStaticMethod_Float_A),
        CreateCallStaticMismatchParam<ani_float>("retInt", ":i", ANI_INVALID_TYPE,
                                                 &ani_env::Class_CallStaticMethod_Float,
                                                 &ani_env::Class_CallStaticMethod_Float_A),
        CreateCallStaticMismatchParam<ani_double>("retFloat", ":f", ANI_INVALID_TYPE,
                                                  &ani_env::Class_CallStaticMethod_Double,
                                                  &ani_env::Class_CallStaticMethod_Double_A)};
}

static std::vector<CallStaticMismatchParam> GetRefAndVoidMismatchParams()
{
    return {
        CreateCallStaticMismatchParam<ani_ref>("retInt", ":i", ANI_INVALID_TYPE, &ani_env::Class_CallStaticMethod_Ref,
                                               &ani_env::Class_CallStaticMethod_Ref_A),
        CreateCallStaticMismatchParam<ani_ref>("retBool", ":z", ANI_INVALID_TYPE, &ani_env::Class_CallStaticMethod_Ref,
                                               &ani_env::Class_CallStaticMethod_Ref_A),
        CreateCallStaticMismatchParam<ani_int>("retRef", ":C{std.core.Array}", ANI_INVALID_TYPE,
                                               &ani_env::Class_CallStaticMethod_Int,
                                               &ani_env::Class_CallStaticMethod_Int_A),
        CreateCallStaticMismatchParam<ani_boolean>("retRef", ":C{std.core.Array}", ANI_INVALID_TYPE,
                                                   &ani_env::Class_CallStaticMethod_Boolean,
                                                   &ani_env::Class_CallStaticMethod_Boolean_A),
        CreateCallStaticMismatchParam<ani_boolean>("retVoid", ":", ANI_INVALID_TYPE,
                                                   &ani_env::Class_CallStaticMethod_Boolean,
                                                   &ani_env::Class_CallStaticMethod_Boolean_A),
        CreateCallStaticMismatchParam<ani_ref>("retVoid", ":", ANI_INVALID_TYPE, &ani_env::Class_CallStaticMethod_Ref,
                                               &ani_env::Class_CallStaticMethod_Ref_A),
        CreateCallStaticMismatchParam<ani_byte>("retVoid", ":", ANI_INVALID_TYPE, &ani_env::Class_CallStaticMethod_Byte,
                                                &ani_env::Class_CallStaticMethod_Byte_A),
        CreateCallStaticMismatchParam<ani_long>("retVoid", ":", ANI_INVALID_TYPE, &ani_env::Class_CallStaticMethod_Long,
                                                &ani_env::Class_CallStaticMethod_Long_A),
        CreateCallStaticMismatchParam<ani_double>("retVoid", ":", ANI_INVALID_TYPE,
                                                  &ani_env::Class_CallStaticMethod_Double,
                                                  &ani_env::Class_CallStaticMethod_Double_A),
        CreateCallStaticMismatchVoidParam("retBool", ":z", ANI_INVALID_TYPE),
        CreateCallStaticMismatchVoidParam("retInt", ":i", ANI_INVALID_TYPE),
        CreateCallStaticMismatchVoidParam("retRef", ":C{std.core.Array}", ANI_INVALID_TYPE)};
}

TEST_F(CallStaticMethodReturnTypeTest, Success)
{
    for (const auto &param : GetSuccessParams()) {
        RunTest(param);
    }
}

TEST_F(CallStaticMethodReturnTypeTest, PrimitiveMismatch)
{
    for (const auto &param : GetPrimitiveMismatchParams()) {
        RunTest(param);
    }
}

TEST_F(CallStaticMethodReturnTypeTest, RefAndVoidMismatch)
{
    for (const auto &param : GetRefAndVoidMismatchParams()) {
        RunTest(param);
    }
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)