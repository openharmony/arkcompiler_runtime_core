/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

namespace panda::ets::interop::js::testing {

class EtsInteropScenariosEtsToJs : public EtsInteropTest {};

TEST_F(EtsInteropScenariosEtsToJs, test_standalone_function_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_standalone_function_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_class_method_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_class_method_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_interface_method_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_interface_method_call.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for eTS getter use in JS
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_class_getter)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_class_getter.js"));
}

// NOTE(oignatenko) enable this after interop is implemented for eTS setter use in JS
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_class_setter)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_class_setter.js"));
}

// NOTE(oignatenko) enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_lambda_function_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_lambda_function_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_generic_function_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_generic_function_call.js"));
}

// NOTE(oignatenko) enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_extend_class)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_extend_class.js"));
}

// NOTE(oignatenko) return and arg types any, unknown, undefined need real TS because transpiling cuts off
// important details. Have a look at declgen_ets2ts
TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_any_call)
{
    // Note this also covers scenario of return type any
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_any_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_unknown_call)
{
    // Note this also covers scenario of return type unknown
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_unknown_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_undefined_call)
{
    // Note this also covers scenario of return type undefined
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_undefined_call.js"));
}

// NOTE(oignatenko) enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_tuple_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_tuple_call.js"));
}

// NOTE(oignatenko) enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_callable_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_callable_call.js"));
}

}  // namespace panda::ets::interop::js::testing
