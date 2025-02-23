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

class ObjectCallMethodRefTest : public AniTest {
public:
    void GetMethodData(ani_object *opsResult, ani_method *concatResult, ani_string *s0Result, ani_string *s1Result)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LStringOperations;", &cls), ANI_OK);

        const char *signature = "Lstd/core/String;Lstd/core/String;:Lstd/core/String;";
        ani_method concat;
        ASSERT_EQ(env_->Class_FindMethod(cls, "concat", signature, &concat), ANI_OK);

        auto ops = CallEtsFunction<ani_ref>("newStringOperations");
        ani_string s0;
        ASSERT_EQ(env_->String_NewUTF8("abc", 3U, &s0), ANI_OK);
        ani_string s1;
        ASSERT_EQ(env_->String_NewUTF8("def", 3U, &s1), ANI_OK);

        *opsResult = static_cast<ani_object>(ops);
        *concatResult = concat;
        *s0Result = s0;
        *s1Result = s1;
    }
};

TEST_F(ObjectCallMethodRefTest, object_call_method_ref)
{
    ani_object ops;
    ani_method concat;
    ani_string s0;
    ani_string s1;
    GetMethodData(&ops, &concat, &s0, &s1);

    ani_ref result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Ref(env_, ops, concat, &result, s0, s1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkConcatOps", s0, s1, result), ANI_TRUE);
}

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_v)
{
    ani_object ops;
    ani_method concat;
    ani_string s0;
    ani_string s1;
    GetMethodData(&ops, &concat, &s0, &s1);

    ani_ref result;
    ASSERT_EQ(env_->Object_CallMethod_Ref(ops, concat, &result, s0, s1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkConcatOps", s0, s1, result), ANI_TRUE);
}

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_a)
{
    ani_object ops;
    ani_method concat;
    ani_string s0;
    ani_string s1;
    GetMethodData(&ops, &concat, &s0, &s1);

    ani_value args[2U];
    args[0U].r = s0;
    args[1U].r = s1;

    ani_ref result;
    ASSERT_EQ(env_->Object_CallMethod_Ref_A(ops, concat, &result, args), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkConcatOps", s0, s1, result), ANI_TRUE);
}

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_invalid_object)
{
    ani_object ops;
    ani_method concat;
    ani_string s0;
    ani_string s1;
    GetMethodData(&ops, &concat, &s0, &s1);

    ani_ref result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Ref(env_, nullptr, concat, &result, s0, s1), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_invalid_method)
{
    ani_object ops;
    ani_method concat;
    ani_string s0;
    ani_string s1;
    GetMethodData(&ops, &concat, &s0, &s1);

    ani_ref result;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Ref(env_, ops, nullptr, &result, s0, s1), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_invalid_result)
{
    ani_object ops;
    ani_method concat;
    ani_string s0;
    ani_string s1;
    GetMethodData(&ops, &concat, &s0, &s1);

    ASSERT_EQ(env_->c_api->Object_CallMethod_Ref(env_, ops, concat, nullptr, s0, s1), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_v_invalid_object)
{
    ani_object ops;
    ani_method concat;
    ani_string s0;
    ani_string s1;
    GetMethodData(&ops, &concat, &s0, &s1);

    ani_ref result;
    ASSERT_EQ(env_->Object_CallMethod_Ref(nullptr, concat, &result, s0, s1), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_v_invalid_method)
{
    ani_object ops;
    ani_method concat;
    ani_string s0;
    ani_string s1;
    GetMethodData(&ops, &concat, &s0, &s1);

    ani_ref result;
    ASSERT_EQ(env_->Object_CallMethod_Ref(ops, nullptr, &result, s0, s1), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_v_invalid_result)
{
    ani_object ops;
    ani_method concat;
    ani_string s0;
    ani_string s1;
    GetMethodData(&ops, &concat, &s0, &s1);

    ASSERT_EQ(env_->Object_CallMethod_Ref(ops, concat, nullptr, s0, s1), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_a_invalid_object)
{
    ani_object ops;
    ani_method concat;
    ani_string s0;
    ani_string s1;
    GetMethodData(&ops, &concat, &s0, &s1);

    ani_value args[2U];
    args[0U].r = s0;
    args[1U].r = s1;

    ani_ref result;
    ASSERT_EQ(env_->Object_CallMethod_Ref_A(nullptr, concat, &result, args), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_a_invalid_method)
{
    ani_object ops;
    ani_method concat;
    ani_string s0;
    ani_string s1;
    GetMethodData(&ops, &concat, &s0, &s1);

    ani_value args[2U];
    args[0U].r = s0;
    args[1U].r = s1;

    ani_ref result;
    ASSERT_EQ(env_->Object_CallMethod_Ref_A(ops, nullptr, &result, args), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_a_invalid_result)
{
    ani_object ops;
    ani_method concat;
    ani_string s0;
    ani_string s1;
    GetMethodData(&ops, &concat, &s0, &s1);

    ani_value args[2U];
    args[0U].r = s0;
    args[1U].r = s1;

    ASSERT_EQ(env_->Object_CallMethod_Ref_A(ops, concat, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_a_invalid_args)
{
    ani_object ops;
    ani_method concat;
    ani_string s0;
    ani_string s1;
    GetMethodData(&ops, &concat, &s0, &s1);

    ani_ref result;
    ASSERT_EQ(env_->Object_CallMethod_Ref_A(ops, concat, &result, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
