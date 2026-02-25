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

struct CallObjectByNameMismatchParam {
    std::string testName;
    std::string etsMethodName;
    std::string etsSignature;
    ani_status expectedStatus;

    std::function<ani_status(ani_env *, ani_object, const char *, const char *)> callV;
    std::function<ani_status(ani_env *, ani_object, const char *, const char *)> callA;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ObjectCallMethodByNameReturnTypeTest : public AniGtestObjectOps {
protected:
    void RunTest(const CallObjectByNameMismatchParam &param)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_call_method_by_name_mismatch_test.TypeProvider", &cls), ANI_OK);

        ani_method ctor {};
        ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);

        ani_object object {};
        ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);
        ASSERT_NE(object, nullptr);

        ani_status statusV = param.callV(env_, object, param.etsMethodName.c_str(), param.etsSignature.c_str());
        EXPECT_EQ(statusV, param.expectedStatus) << "Failed Object_CallMethodByName: " << param.testName;

        ani_status statusA = param.callA(env_, object, param.etsMethodName.c_str(), param.etsSignature.c_str());
        EXPECT_EQ(statusA, param.expectedStatus) << "Failed Object_CallMethodByName_A: " << param.testName;
    }
};

template <typename T, typename FuncV, typename FuncA>
CallObjectByNameMismatchParam CreateCallObjectByNameMismatchParam(const std::string &etsMethodName,
                                                                  const std::string &sig, ani_status expectedStatus,
                                                                  FuncV callV, FuncA callA)
{
    auto callByNameV = [callV](ani_env *env, ani_object o, const char *name, const char *s) -> ani_status {
        T res;
        return (env->*callV)(o, name, s, &res);
    };
    auto callByNameA = [callA](ani_env *env, ani_object o, const char *name, const char *s) -> ani_status {
        T res;
        ani_value args[1] {};
        return (env->*callA)(o, name, s, &res, args);
    };

    return CallObjectByNameMismatchParam {"Call" + etsMethodName, etsMethodName,          sig,
                                          expectedStatus,         std::move(callByNameV), std::move(callByNameA)};
}

CallObjectByNameMismatchParam CreateCallObjectByNameMismatchVoidParam(const std::string &etsMethodName,
                                                                      const std::string &sig, ani_status status)
{
    auto callByNameVoidV = [](ani_env *env, ani_object o, const char *name, const char *s) -> ani_status {
        return env->Object_CallMethodByName_Void(o, name, s);
    };
    auto callByNameVoidA = [](ani_env *env, ani_object o, const char *name, const char *s) -> ani_status {
        ani_value args[1] {};
        return env->Object_CallMethodByName_Void_A(o, name, s, args);
    };

    return CallObjectByNameMismatchParam {
        "VoidCall" + etsMethodName, etsMethodName, sig, status, std::move(callByNameVoidV), std::move(callByNameVoidA)};
}

static std::vector<CallObjectByNameMismatchParam> GetObjectCallByNameSuccessParams()
{
    return {
        CreateCallObjectByNameMismatchParam<ani_boolean>("retBool", ":z", ANI_OK,
                                                         &ani_env::Object_CallMethodByName_Boolean,
                                                         &ani_env::Object_CallMethodByName_Boolean_A),
        CreateCallObjectByNameMismatchParam<ani_byte>("retByte", ":b", ANI_OK, &ani_env::Object_CallMethodByName_Byte,
                                                      &ani_env::Object_CallMethodByName_Byte_A),
        CreateCallObjectByNameMismatchParam<ani_char>("retChar", ":c", ANI_OK, &ani_env::Object_CallMethodByName_Char,
                                                      &ani_env::Object_CallMethodByName_Char_A),
        CreateCallObjectByNameMismatchParam<ani_short>("retShort", ":s", ANI_OK,
                                                       &ani_env::Object_CallMethodByName_Short,
                                                       &ani_env::Object_CallMethodByName_Short_A),
        CreateCallObjectByNameMismatchParam<ani_int>("retInt", ":i", ANI_OK, &ani_env::Object_CallMethodByName_Int,
                                                     &ani_env::Object_CallMethodByName_Int_A),
        CreateCallObjectByNameMismatchParam<ani_long>("retLong", ":l", ANI_OK, &ani_env::Object_CallMethodByName_Long,
                                                      &ani_env::Object_CallMethodByName_Long_A),
        CreateCallObjectByNameMismatchParam<ani_float>("retFloat", ":f", ANI_OK,
                                                       &ani_env::Object_CallMethodByName_Float,
                                                       &ani_env::Object_CallMethodByName_Float_A),
        CreateCallObjectByNameMismatchParam<ani_double>("retDouble", ":d", ANI_OK,
                                                        &ani_env::Object_CallMethodByName_Double,
                                                        &ani_env::Object_CallMethodByName_Double_A),
        CreateCallObjectByNameMismatchParam<ani_ref>("retRef", ":C{std.core.Array}", ANI_OK,
                                                     &ani_env::Object_CallMethodByName_Ref,
                                                     &ani_env::Object_CallMethodByName_Ref_A),
        CreateCallObjectByNameMismatchVoidParam("retVoid", ":", ANI_OK)};
}

static std::vector<CallObjectByNameMismatchParam> GetObjectCallByNameTypeMismatchParams()
{
    return {CreateCallObjectByNameMismatchParam<ani_int>("retBool", ":z", ANI_INVALID_TYPE,
                                                         &ani_env::Object_CallMethodByName_Int,
                                                         &ani_env::Object_CallMethodByName_Int_A),
            CreateCallObjectByNameMismatchParam<ani_boolean>("retInt", ":i", ANI_INVALID_TYPE,
                                                             &ani_env::Object_CallMethodByName_Boolean,
                                                             &ani_env::Object_CallMethodByName_Boolean_A),
            CreateCallObjectByNameMismatchParam<ani_char>("retShort", ":s", ANI_INVALID_TYPE,
                                                          &ani_env::Object_CallMethodByName_Char,
                                                          &ani_env::Object_CallMethodByName_Char_A),
            CreateCallObjectByNameMismatchParam<ani_byte>("retChar", ":c", ANI_INVALID_TYPE,
                                                          &ani_env::Object_CallMethodByName_Byte,
                                                          &ani_env::Object_CallMethodByName_Byte_A),
            CreateCallObjectByNameMismatchParam<ani_ref>("retLong", ":l", ANI_INVALID_TYPE,
                                                         &ani_env::Object_CallMethodByName_Ref,
                                                         &ani_env::Object_CallMethodByName_Ref_A),
            CreateCallObjectByNameMismatchParam<ani_long>("retRef", ":C{std.core.Array}", ANI_INVALID_TYPE,
                                                          &ani_env::Object_CallMethodByName_Long,
                                                          &ani_env::Object_CallMethodByName_Long_A)};
}

static std::vector<CallObjectByNameMismatchParam> GetObjectCallByNameVoidMismatchParams()
{
    return {CreateCallObjectByNameMismatchVoidParam("retInt", ":i", ANI_INVALID_TYPE),
            CreateCallObjectByNameMismatchVoidParam("retChar", ":c", ANI_INVALID_TYPE),
            CreateCallObjectByNameMismatchVoidParam("retRef", ":C{std.core.Array}", ANI_INVALID_TYPE),
            CreateCallObjectByNameMismatchParam<ani_boolean>("retVoid", ":", ANI_INVALID_TYPE,
                                                             &ani_env::Object_CallMethodByName_Boolean,
                                                             &ani_env::Object_CallMethodByName_Boolean_A),
            CreateCallObjectByNameMismatchParam<ani_int>("retVoid", ":", ANI_INVALID_TYPE,
                                                         &ani_env::Object_CallMethodByName_Int,
                                                         &ani_env::Object_CallMethodByName_Int_A),
            CreateCallObjectByNameMismatchParam<ani_ref>("retVoid", ":", ANI_INVALID_TYPE,
                                                         &ani_env::Object_CallMethodByName_Ref,
                                                         &ani_env::Object_CallMethodByName_Ref_A)};
}

TEST_F(ObjectCallMethodByNameReturnTypeTest, Success)
{
    for (const auto &param : GetObjectCallByNameSuccessParams()) {
        RunTest(param);
    }
}

TEST_F(ObjectCallMethodByNameReturnTypeTest, TypeMismatch)
{
    for (const auto &param : GetObjectCallByNameTypeMismatchParams()) {
        RunTest(param);
    }
}

TEST_F(ObjectCallMethodByNameReturnTypeTest, VoidMismatch)
{
    for (const auto &param : GetObjectCallByNameVoidMismatchParams()) {
        RunTest(param);
    }
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)