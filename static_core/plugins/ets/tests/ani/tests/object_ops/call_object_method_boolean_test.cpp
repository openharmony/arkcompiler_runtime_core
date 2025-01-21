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

namespace ark::ets::ani::testing {

/**
 * @brief Unit test class for testing boolean method calls on ani objects.
 *
 * Inherits from the AniTest base class and provides test cases to verify
 * correct functionality of calling boolean-returning methods with various
 * parameter scenarios.
 */
class CallObjectMethodBooleanTest : public AniTest {
public:
    /**
     * @brief Retrieves the ani_object and ani_method needed for testing.
     *
     * This method allocates an object and fetches a method from the class,
     * which can then be used in test cases for method invocation.
     *
     * @param objectResult Pointer to store the allocated ani_object.
     * @param methodResult Pointer to store the retrieved ani_method.
     */
    void GetMethodData(ani_object *objectResult, ani_method *methodResult)  // NOLINT(readability-identifier-naming)
    {
        ani_class cls;
        // Locate the class "LA;" in the environment.
        ASSERT_EQ(env_->FindClass("LA;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        // Emulate allocation an instance of class.
        ani_static_method newMethod;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env_->Class_GetStaticMethod(cls, "new_A", ":LA;", &newMethod), ANI_OK);
        ani_ref ref;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        ani_method method;
        // Retrieve a method named "boolean_method" with signature "II:Z".
        ASSERT_EQ(env_->Class_GetMethod(cls, "boolean_method", "II:Z", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *objectResult = static_cast<ani_object>(ref);
        *methodResult = method;
    }
};

/**
 * @brief Test case for calling a boolean-returning method with an argument array.
 *
 * This test verifies the correct behavior of calling a method using an array
 * of integer arguments and checks the return value.
 */
TEST_F(CallObjectMethodBooleanTest, object_call_method_boolean_a)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_value args[2];  // NOLINT(modernize-avoid-c-arrays)
    ani_int arg1 = 2U;
    ani_int arg2 = 3U;
    args[0].i = arg1;
    args[1].i = arg2;

    ani_boolean res;
    // Call the method and verify the return value.
    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &res, args), ANI_OK);
    ASSERT_EQ(res, false);
}

/**
 * @brief Test case for calling a boolean-returning method with variadic arguments.
 *
 * This test ensures that the method correctly handles variadic arguments and
 * produces the expected boolean result.
 */
TEST_F(CallObjectMethodBooleanTest, object_call_method_boolean_v)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_boolean res;
    ani_int arg1 = 5U;
    ani_int arg2 = 6U;
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
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_boolean res;
    ani_int arg1 = 2U;
    ani_int arg2 = 3U;
    // Call the method and verify the return value.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean(env_, object, method, &res, arg1, arg2), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

/**
 * @brief Test case for handling invalid method pointer during invocation.
 *
 * Ensures the method call returns the expected error when a nullptr is passed
 * as the method pointer.
 */
TEST_F(CallObjectMethodBooleanTest, call_method_boolean_v_invalid_method)
{
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_boolean res;
    ani_int arg1 = 5U;
    ani_int arg2 = 6U;
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
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_int arg1 = 5U;
    ani_int arg2 = 6U;
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
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_boolean res;
    ani_int arg1 = 5U;
    ani_int arg2 = 6U;
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
    ani_object object;
    ani_method method;
    GetMethodData(&object, &method);

    ani_boolean res;
    // Attempt to call the method with a nullptr argument array.
    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(nullptr, method, &res, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
