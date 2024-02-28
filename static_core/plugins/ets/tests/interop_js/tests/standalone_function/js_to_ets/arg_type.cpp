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

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_double)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeDouble");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_byte)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeByte");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_short)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeShort");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_int)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeInt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_long)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeLong");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_float)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeFloat");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_char)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeChar");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_boolean)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeBoolean");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_string)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeString");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_object)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeObject");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_class)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeClass");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_array)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeArray");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_tuple)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeTuple");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_callable)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeCallable");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_double_lit)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeDoubleLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_byte_lit)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeByteLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_short_lit)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeShortLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_int_lit)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeIntLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_long_lit)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeLongLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_float_lit)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeFloatLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_char_lit)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeCharLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_boolean_lit)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeBooleanLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_string_lit)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeStringLit");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
