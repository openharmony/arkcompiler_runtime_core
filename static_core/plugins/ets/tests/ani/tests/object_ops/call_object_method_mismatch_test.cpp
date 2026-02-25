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

#include "ani_gtest_object_ops.h"
#include <vector>
#include <string>
#include <functional>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

struct CallObjectMismatchParam {
    std::string testName;
    std::string etsMethodName;
    std::string etsSignature;
    ani_status expectedStatus;

    std::function<ani_status(ani_env *, ani_object, ani_method)> callV;
    std::function<ani_status(ani_env *, ani_object, ani_method)> callA;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ObjectCallMethodReturnTypeTest : public AniGtestObjectOps {
protected:
    void RunTest(const CallObjectMismatchParam &param)
    {
        ani_object object {};
        ani_method method {};

        GetMethodAndObject("call_object_method_mismatch_test.TypeProvider", param.etsMethodName.c_str(),
                           param.etsSignature.c_str(), &object, &method);

        ani_status statusV = param.callV(env_, object, method);
        EXPECT_EQ(statusV, param.expectedStatus) << "Failed Object_CallMethod: " << param.testName;

        ani_status statusA = param.callA(env_, object, method);
        EXPECT_EQ(statusA, param.expectedStatus) << "Failed Object_CallMethod_A: " << param.testName;
    }
};

template <typename T, typename FuncV, typename FuncA>
CallObjectMismatchParam CreateCallObjectMismatchParam(const std::string &etsMethodName, const std::string &sig,
                                                      ani_status expectedStatus, FuncV callV, FuncA callA)
{
    auto callObjV = [callV](ani_env *env, ani_object o, ani_method m) -> ani_status {
        T res;
        return (env->*callV)(o, m, &res);
    };
    auto callObjA = [callA](ani_env *env, ani_object o, ani_method m) -> ani_status {
        T res;
        ani_value args[1] {};
        return (env->*callA)(o, m, &res, args);
    };

    return CallObjectMismatchParam {"Call" + etsMethodName, etsMethodName,       sig,
                                    expectedStatus,         std::move(callObjV), std::move(callObjA)};
}

CallObjectMismatchParam CreateCallObjectMismatchVoidParam(const std::string &etsMethodName, const std::string &sig,
                                                          ani_status status)
{
    auto callObjVoidV = [](ani_env *env, ani_object o, ani_method m) -> ani_status {
        return env->Object_CallMethod_Void(o, m);
    };
    auto callObjVoidA = [](ani_env *env, ani_object o, ani_method m) -> ani_status {
        ani_value args[1] {};
        return env->Object_CallMethod_Void_A(o, m, args);
    };

    return CallObjectMismatchParam {"VoidCall" + etsMethodName, etsMethodName,          sig, status,
                                    std::move(callObjVoidV),    std::move(callObjVoidA)};
}

static std::vector<CallObjectMismatchParam> GetObjectCallSuccessParams()
{
    return {CreateCallObjectMismatchParam<ani_boolean>("retBool", ":z", ANI_OK, &ani_env::Object_CallMethod_Boolean,
                                                       &ani_env::Object_CallMethod_Boolean_A),
            CreateCallObjectMismatchParam<ani_byte>("retByte", ":b", ANI_OK, &ani_env::Object_CallMethod_Byte,
                                                    &ani_env::Object_CallMethod_Byte_A),
            CreateCallObjectMismatchParam<ani_char>("retChar", ":c", ANI_OK, &ani_env::Object_CallMethod_Char,
                                                    &ani_env::Object_CallMethod_Char_A),
            CreateCallObjectMismatchParam<ani_short>("retShort", ":s", ANI_OK, &ani_env::Object_CallMethod_Short,
                                                     &ani_env::Object_CallMethod_Short_A),
            CreateCallObjectMismatchParam<ani_int>("retInt", ":i", ANI_OK, &ani_env::Object_CallMethod_Int,
                                                   &ani_env::Object_CallMethod_Int_A),
            CreateCallObjectMismatchParam<ani_long>("retLong", ":l", ANI_OK, &ani_env::Object_CallMethod_Long,
                                                    &ani_env::Object_CallMethod_Long_A),
            CreateCallObjectMismatchParam<ani_float>("retFloat", ":f", ANI_OK, &ani_env::Object_CallMethod_Float,
                                                     &ani_env::Object_CallMethod_Float_A),
            CreateCallObjectMismatchParam<ani_double>("retDouble", ":d", ANI_OK, &ani_env::Object_CallMethod_Double,
                                                      &ani_env::Object_CallMethod_Double_A),
            CreateCallObjectMismatchParam<ani_ref>("retRef", ":C{std.core.Array}", ANI_OK,
                                                   &ani_env::Object_CallMethod_Ref, &ani_env::Object_CallMethod_Ref_A),
            CreateCallObjectMismatchVoidParam("retVoid", ":", ANI_OK)};
}

static std::vector<CallObjectMismatchParam> GetObjectCallTypeMismatchParams()
{
    return {
        CreateCallObjectMismatchParam<ani_boolean>("retInt", ":i", ANI_INVALID_TYPE,
                                                   &ani_env::Object_CallMethod_Boolean,
                                                   &ani_env::Object_CallMethod_Boolean_A),
        CreateCallObjectMismatchParam<ani_int>("retChar", ":c", ANI_INVALID_TYPE, &ani_env::Object_CallMethod_Int,
                                               &ani_env::Object_CallMethod_Int_A),
        CreateCallObjectMismatchParam<ani_char>("retByte", ":b", ANI_INVALID_TYPE, &ani_env::Object_CallMethod_Char,
                                                &ani_env::Object_CallMethod_Char_A),
        CreateCallObjectMismatchParam<ani_byte>("retShort", ":s", ANI_INVALID_TYPE, &ani_env::Object_CallMethod_Byte,
                                                &ani_env::Object_CallMethod_Byte_A),
        CreateCallObjectMismatchParam<ani_ref>("retLong", ":l", ANI_INVALID_TYPE, &ani_env::Object_CallMethod_Ref,
                                               &ani_env::Object_CallMethod_Ref_A),
        CreateCallObjectMismatchParam<ani_long>("retRef", ":C{std.core.Array}", ANI_INVALID_TYPE,
                                                &ani_env::Object_CallMethod_Long, &ani_env::Object_CallMethod_Long_A)};
}

static std::vector<CallObjectMismatchParam> GetObjectCallVoidMismatchParams()
{
    return {CreateCallObjectMismatchVoidParam("retBool", ":z", ANI_INVALID_TYPE),
            CreateCallObjectMismatchVoidParam("retInt", ":i", ANI_INVALID_TYPE),
            CreateCallObjectMismatchVoidParam("retRef", ":C{std.core.Array}", ANI_INVALID_TYPE),
            CreateCallObjectMismatchParam<ani_boolean>("retVoid", ":", ANI_INVALID_TYPE,
                                                       &ani_env::Object_CallMethod_Boolean,
                                                       &ani_env::Object_CallMethod_Boolean_A),
            CreateCallObjectMismatchParam<ani_int>("retVoid", ":", ANI_INVALID_TYPE, &ani_env::Object_CallMethod_Int,
                                                   &ani_env::Object_CallMethod_Int_A),
            CreateCallObjectMismatchParam<ani_double>("retVoid", ":", ANI_INVALID_TYPE,
                                                      &ani_env::Object_CallMethod_Double,
                                                      &ani_env::Object_CallMethod_Double_A)};
}

TEST_F(ObjectCallMethodReturnTypeTest, Success)
{
    for (const auto &param : GetObjectCallSuccessParams()) {
        RunTest(param);
    }
}

TEST_F(ObjectCallMethodReturnTypeTest, TypeMismatch)
{
    for (const auto &param : GetObjectCallTypeMismatchParams()) {
        RunTest(param);
    }
}

TEST_F(ObjectCallMethodReturnTypeTest, VoidMismatch)
{
    for (const auto &param : GetObjectCallVoidMismatchParams()) {
        RunTest(param);
    }
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)