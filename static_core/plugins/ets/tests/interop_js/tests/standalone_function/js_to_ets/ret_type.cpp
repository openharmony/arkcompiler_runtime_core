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

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_double)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeDouble");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_byte)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeByte");
    ASSERT_EQ(ret, true);
}

// NOTE #18602 (nikitayegorov) enable when numeric type casting is fixed for JS-calls-Ark interoperability
TEST_F(EtsInteropScenariosJsToEts, DISABLED_Test_function_return_type_short)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeShort");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_int)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeInt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_long)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeLong");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_float)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeFloat");
    ASSERT_EQ(ret, true);
}

// NOTE #19091 (nikitayegorov) enable when casting to Ark char type is fixed
TEST_F(EtsInteropScenariosJsToEts, DISABLED_Test_function_return_type_char)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeChar");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_boolean)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeBoolean");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_string)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeString");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_object)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeObject");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_class)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeClass");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_array)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeArray");
    ASSERT_EQ(ret, true);
}

// NOTE #19087 (nikitayegorov) enable when passing callables is fixed
TEST_F(EtsInteropScenariosJsToEts, DISABLED_Test_function_return_type_callable)
{
    auto ret = CallEtsMethod<bool>("TestFunctionReturnTypeCallable");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
