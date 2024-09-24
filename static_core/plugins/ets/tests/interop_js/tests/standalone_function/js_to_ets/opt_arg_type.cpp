/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_double_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeDoubleOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_byte_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeByteOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_short_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeShortOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_int_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeIntOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_long_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeLongOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_float_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeFloatOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_char_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeCharOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_boolean_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeBooleanOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_string_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeStringOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_object_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeObjectOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_class_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeClassOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_array_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeArrayOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_tuple_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeTupleOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_callable_opt)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeCallableOpt");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
