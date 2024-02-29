/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsInteropScenariosJsToEts : public EtsInteropTest {};

TEST_F(EtsInteropScenariosJsToEts, test_standalone_function_call)
{
    auto ret = CallEtsMethod<bool>("TestStandaloneFunctionCall");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, test_class_methodCall)
{
    auto ret = CallEtsMethod<bool>("TestClassMethodCall");
    ASSERT_EQ(ret, true);
}

[[maybe_unused]] constexpr int CAN_CREATE_CLASSES_AT_RUNTIME = 0;
#if CAN_CREATE_CLASSES_AT_RUNTIME
TEST_F(EtsInteropScenariosJsToEts, Test_interface_methodCall)
{
    auto ret = CallEtsMethod<bool>("TestInterfaceMethodCall");
    ASSERT_EQ(ret, true);
}
#endif

TEST_F(EtsInteropScenariosJsToEts, Test_class_getter)
{
    auto ret = CallEtsMethod<bool>("TestClassGetter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_class_setter)
{
    auto ret = CallEtsMethod<bool>("TestClassSetter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_lambda_function_call)
{
    auto ret = CallEtsMethod<bool>("TestLambdaFunctionCall");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_generic_function_call)
{
    auto ret = CallEtsMethod<bool>("TestGenericFunctionCall");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_any)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_unknown)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeUnknown");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_undefined)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeUndefined");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_tuple)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeTuple");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_any)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_unknown)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeUnknown");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_undefined)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeUndefined");
    ASSERT_EQ(ret, true);
}

// NOTE #15891 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEts, DISABLED_Test_function_arg_type_callable)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeCallable");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_default_value_define_for_parameter)
{
    auto ret = CallEtsMethod<bool>("TestDefaultValueDefineForParameter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_default_value_int_define_for_parameter)
{
    auto ret = CallEtsMethod<bool>("TestDefaultValueIntDefineForParameter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_generic_type_as_parameter)
{
    auto ret = CallEtsMethod<bool>("TestGenericTypeAsParameter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_generic_type_as_return_value)
{
    auto ret = CallEtsMethod<bool>("TestGenericTypeAsReturnValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_optional_primitive_explicit)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeOptionalPrimitiveExplicit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_optional_primitive_default)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeOptionalPrimitiveDefault");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_primitive)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypePrimitive");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_primitive)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypePrimitive");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_optional_call)
{
    auto ret = CallEtsMethod<bool>("testOptionals");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_rest_params)
{
    auto ret = CallEtsMethod<bool>("TestRestParams");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_union)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeUnion");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_union)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeUnion");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_class_method_arg_type_union)
{
    auto ret = CallEtsMethod<bool>("TestClassMethodArgTypeUnion");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_class_method_return_type_union)
{
    auto ret = CallEtsMethod<bool>("TestClassMethodReturnTypeUnion");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_interface_methodCall_return_type_union)
{
    auto ret = CallEtsMethod<bool>("TestInterfaceMethodCallReturnTypeUnion");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_static_methodCall)
{
    auto ret = CallEtsMethod<bool>("TestStaticMethodCall");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_static_methodCall_return_type_union)
{
    auto ret = CallEtsMethod<bool>("TestStaticMethodCallReturnTypeUnion");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_rest_parameter)
{
    auto ret = CallEtsMethod<bool>("testFunctionRestParameter");
    ASSERT_EQ(ret, true);
}

// NOTE(nikitayegorov) #17339 enable after rest\spread is fixed
TEST_F(EtsInteropScenariosJsToEts, DISABLED_Test_function_spread_parameter)
{
    auto ret = CallEtsMethod<bool>("testFunctionSpreadParameter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, testFunctionOverload)
{
    auto ret = CallEtsMethod<bool>("testFunctionOverload");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, testFunctionCallableReturnValue)
{
    auto ret = CallEtsMethod<bool>("testFunctionCallableReturnValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, testFunctionArgStringLiteralType)
{
    auto ret = CallEtsMethod<bool>("testFunctionArgStringLiteralType");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, testFunctionIntersectionTypePrimitive)
{
    auto ret = CallEtsMethod<bool>("testFunctionIntersectionTypePrimitive");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_class_in_place_field_declarations)
{
    auto ret = CallEtsMethod<bool>("testClassInPlaceFieldDeclarations");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_returns_composite_type_int)
{
    auto ret = CallEtsMethod<bool>("testFunctionReturnsCompositeTypeInt");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
