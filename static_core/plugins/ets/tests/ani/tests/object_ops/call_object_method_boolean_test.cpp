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

#include "ani_gtest_object_ops.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

/**
 * @brief Unit test class for testing boolean method calls on ani objects.
 *
 * Inherits from the AniTest base class and provides test cases to verify
 * correct functionality of calling boolean-returning methods with various
 * parameter scenarios.
 */
class CallObjectMethodBooleanTest : public AniGtestObjectOps {
public:
    static constexpr ani_int VAL1 = 5U;
    static constexpr ani_int VAL2 = 6U;
    static constexpr ani_int VAL3 = 2U;
    static constexpr ani_int VAL4 = 3U;
};

/**
 * @brief Test case for calling a boolean-returning method with an argument array.
 *
 * This test verifies the correct behavior of calling a method using an array
 * of integer arguments and checks the return value.
 */
TEST_F(CallObjectMethodBooleanTest, object_call_method_boolean_a)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/A;", "booleanMethod", "II:Z", &object, &method);

    ani_value args[2];  // NOLINT(modernize-avoid-c-arrays)
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    args[0].i = arg1;
    args[1].i = arg2;

    ani_boolean res = ANI_FALSE;
    // Call the method and verify the return value.
    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

/**
 * @brief Test case for calling a boolean-returning method with variadic arguments.
 *
 * This test ensures that the method correctly handles variadic arguments and
 * produces the expected boolean result.
 */
TEST_F(CallObjectMethodBooleanTest, object_call_method_boolean_v)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/A;", "booleanMethod", "II:Z", &object, &method);

    ani_boolean res = ANI_FALSE;
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    // Call the method using variadic arguments and verify the return value.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &res, arg1, arg2), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

/**
 * @brief Test case for calling a boolean-returning method with specific inputs.
 *
 * Verifies the functionality of calling a method using variadic arguments with
 * different inputs and checking the boolean return value.
 */
TEST_F(CallObjectMethodBooleanTest, object_call_method_boolean)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/A;", "booleanMethod", "II:Z", &object, &method);

    ani_boolean res = ANI_FALSE;
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    // Call the method and verify the return value.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean(env_, object, method, &res, arg1, arg2), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanTest, object_call_method_boolean_v_invalid_env)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/A;", "booleanMethod", "II:Z", &object, &method);

    ani_boolean res = ANI_FALSE;
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    // Call the method using variadic arguments and verify the return value.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &res, arg1, arg2), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

/**
 * @brief Test case for handling invalid method pointer during invocation.
 *
 * Ensures the method call returns the expected error when a nullptr is passed
 * as the method pointer.
 */
TEST_F(CallObjectMethodBooleanTest, call_method_boolean_v_invalid_method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/A;", "booleanMethod", "II:Z", &object, &method);

    ani_boolean res = ANI_FALSE;
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    // Attempt to call the method with a nullptr method pointer.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, nullptr, &res, arg1, arg2), ANI_INVALID_ARGS);
}

/**
 * @brief Test case for handling null result pointers during method invocation.
 *
 * Ensures the method call returns the expected error when the result pointer is null.
 */
TEST_F(CallObjectMethodBooleanTest, call_method_boolean_v_invalid_result)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/A;", "booleanMethod", "II:Z", &object, &method);

    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    // Attempt to call the method with a nullptr result pointer.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, nullptr, arg1, arg2), ANI_INVALID_ARGS);
}

/**
 * @brief Test case for handling invalid object pointers during method invocation.
 *
 * Ensures the method call returns the expected error when a nullptr is passed
 * as the object pointer.
 */
TEST_F(CallObjectMethodBooleanTest, call_method_boolean_v_invalid_object)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/A;", "booleanMethod", "II:Z", &object, &method);

    ani_boolean res;
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    // Attempt to call the method with a nullptr object pointer.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Boolean(nullptr, method, &res, arg1, arg2), ANI_INVALID_ARGS);
}

/**
 * @brief Test case for handling invalid argument arrays during method invocation.
 *
 * Ensures the method call returns the expected error when the argument array is null.
 */
TEST_F(CallObjectMethodBooleanTest, call_method_boolean_a_invalid_args)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/A;", "booleanMethod", "II:Z", &object, &method);

    ani_boolean res;
    // Attempt to call the method with a nullptr argument array.
    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &res, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanTest, call_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/A;", "booleanMethodVoidParam", ":Z", &object, &method);

    ani_boolean result = 0U;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanTest, call_Multiple_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/A;", "booleanMethodMultipleParam", "BFFB:Z", &object, &method);

    ani_value args[4U] = {};
    ani_byte arg1 = VAL3;
    ani_float arg2 = 2.0F;
    ani_float arg3 = 3.0F;
    ani_byte arg4 = 4U;
    args[0U].b = arg1;
    args[1U].f = arg2;
    args[2U].f = arg3;
    args[3U].b = arg4;

    ani_boolean result;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result, arg1, arg2, arg3, arg4), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);

    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanTest, call_Parent_Class_Void_Param_Method_1)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/B;", "booleanMethodVoidParam", ":Z", &object, &method);

    ani_boolean result = 0U;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanTest, call_Parent_Class_Method)
{
    ani_class clsC {};
    ASSERT_EQ(env_->FindClass("Lcall_object_method_boolean_test/C;", &clsC), ANI_OK);
    ASSERT_NE(clsC, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(clsC, "func", "II:Z", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_class clsD {};
    ASSERT_EQ(env_->FindClass("Lcall_object_method_boolean_test/D;", &clsD), ANI_OK);
    ASSERT_NE(clsD, nullptr);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(clsD, "<ctor>", ":V", &ctor), ANI_OK);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(clsD, ctor, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_boolean result = ANI_TRUE;
    ani_value args[2U] = {};
    ani_int arg1 = VAL3;
    ani_int arg2 = VAL4;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);

    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanTest, call_Sub_Class_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/E;", "func", "II:Z", &object, &method);

    ani_boolean result = ANI_FALSE;
    ani_value args[2U] = {};
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanTest, multiple_Call_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/A;", "booleanMethod", "II:Z", &object, &method);

    ani_boolean result = ANI_FALSE;
    ani_value args[2U] = {};
    ani_byte arg1 = 6U;
    ani_byte arg2 = 7U;
    args[0U].b = arg1;
    args[1U].b = arg2;

    for (ani_int i = 0; i < VAL4; i++) {
        ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result, arg1, arg2), ANI_OK);
        ASSERT_EQ(result, ANI_TRUE);
        ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
        ASSERT_EQ(result, ANI_TRUE);
    }
}

TEST_F(CallObjectMethodBooleanTest, call_Nested_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/A;", "nestedMethod", nullptr, &object, &method);

    ani_boolean result = ANI_TRUE;
    ani_value args[2U] = {};
    ani_byte arg1 = VAL4;
    ani_byte arg2 = VAL4;
    args[0U].b = arg1;
    args[1U].b = arg2;

    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);

    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanTest, call_Recursion_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("Lcall_object_method_boolean_test/A;", "recursionMethod", "I:Z", &object, &method);

    ani_boolean result = ANI_FALSE;
    ani_value args[1U] = {};
    ani_byte arg1 = VAL4;
    args[0U].i = arg1;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result, arg1), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
