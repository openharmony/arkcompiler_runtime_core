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
    auto ret = CallEtsMethod<bool>("Test_standalone_function_call");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, test_class_method_call)
{
    auto ret = CallEtsMethod<bool>("Test_class_method_call");
    ASSERT_EQ(ret, true);
}

[[maybe_unused]] constexpr int CAN_CREATE_CLASSES_AT_RUNTIME = 0;
#if CAN_CREATE_CLASSES_AT_RUNTIME
TEST_F(EtsInteropScenariosJsToEts, Test_interface_method_call)
{
    auto ret = CallEtsMethod<bool>("Test_interface_method_call");
    ASSERT_EQ(ret, true);
}
#endif

TEST_F(EtsInteropScenariosJsToEts, Test_class_getter)
{
    auto ret = CallEtsMethod<bool>("Test_class_getter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_class_setter)
{
    auto ret = CallEtsMethod<bool>("Test_class_setter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_lambda_function_call)
{
    auto ret = CallEtsMethod<bool>("Test_lambda_function_call");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_generic_function_call)
{
    auto ret = CallEtsMethod<bool>("Test_generic_function_call");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_any)
{
    auto ret = CallEtsMethod<bool>("Test_function_arg_type_any");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_unknown)
{
    auto ret = CallEtsMethod<bool>("Test_function_arg_type_unknown");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_undefined)
{
    auto ret = CallEtsMethod<bool>("Test_function_arg_type_undefined");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_tuple)
{
    auto ret = CallEtsMethod<bool>("Test_function_arg_type_tuple");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_any)
{
    auto ret = CallEtsMethod<bool>("Test_function_return_type_any");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_unknown)
{
    auto ret = CallEtsMethod<bool>("Test_function_return_type_unknown");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_undefined)
{
    auto ret = CallEtsMethod<bool>("Test_function_return_type_undefined");
    ASSERT_EQ(ret, true);
}

// NOTE #15891 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEts, DISABLED_Test_function_arg_type_callable)
{
    auto ret = CallEtsMethod<bool>("Test_function_arg_type_callable");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_default_value_define_for_parameter)
{
    auto ret = CallEtsMethod<bool>("Test_default_value_define_for_parameter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_default_value_int_define_for_parameter)
{
    auto ret = CallEtsMethod<bool>("Test_default_value_int_define_for_parameter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_generic_type_as_parameter)
{
    auto ret = CallEtsMethod<bool>("Test_generic_type_as_parameter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_generic_type_as_return_value)
{
    auto ret = CallEtsMethod<bool>("Test_generic_type_as_return_value");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_optional_primitive_explicit)
{
    auto ret = CallEtsMethod<bool>("Test_function_arg_type_optional_primitive_explicit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_optional_primitive_default)
{
    auto ret = CallEtsMethod<bool>("Test_function_arg_type_optional_primitive_default");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_primitive)
{
    auto ret = CallEtsMethod<bool>("Test_function_arg_type_primitive");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_primitive)
{
    auto ret = CallEtsMethod<bool>("Test_function_return_type_primitive");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_optional_call)
{
    auto ret = CallEtsMethod<bool>("testOptionals");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
