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

class EtsInteropJsIntrinsTest : public EtsInteropTest {
public:
    void SetUp() override
    {
        EtsInteropTest::SetUp();
        LoadModuleAs("test_intrins", "test_intrins/test_intrins.js");
    }
};

TEST_F(EtsInteropJsIntrinsTest, basic1)
{
    constexpr double ARG0 = 271;
    constexpr double ARG1 = 314;
    constexpr double RES = ARG0 + ARG1;
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<double>("js_sum_wrapper_num_num", ARG0, ARG1);
    ASSERT_EQ(ret, RES);
}

TEST_F(EtsInteropJsIntrinsTest, basic2)
{
    constexpr double ARG0 = 12.34;
    constexpr std::string_view ARG1 = "foo";
    constexpr std::string_view RES = "12.34foo";
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<std::string>("js_sum_wrapper_num_str", ARG0, ARG1);
    ASSERT_EQ(ret, RES);
}

TEST_F(EtsInteropJsIntrinsTest, test_convertors)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("test_undefined"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_null"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_boolean"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_number"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_string"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_object"));
    // NOTE(vpukhov): symbol, function, external, bigint

    ASSERT_EQ(true, CallEtsMethod<bool>("test_string_ops"));
}

TEST_F(EtsInteropJsIntrinsTest, test_builtin_array_convertors)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("test_builtin_array_any"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_builtin_array_boolean"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_builtin_array_int"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_builtin_array_number"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_builtin_array_string"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_builtin_array_object"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_builtin_array_multidim"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_builtin_array_instanceof"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_init_array_component"));
}

TEST_F(EtsInteropJsIntrinsTest, test_named_access)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("test_named_access"));
}

TEST_F(EtsInteropJsIntrinsTest, test_newcall)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("test_newcall"));
}

TEST_F(EtsInteropJsIntrinsTest, test_value_call)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("test_value_call"));
}

TEST_F(EtsInteropJsIntrinsTest, test_call_stress)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("test_call_stress"));
}

// Remove after JSValue cast fix
TEST_F(EtsInteropJsIntrinsTest, DISABLED_test_undefined_cast_bug)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("test_undefined_cast_bug"));
}

TEST_F(EtsInteropJsIntrinsTest, test_lambda_proxy)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("test_lambda_proxy"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_lambda_proxy_recursive"));
}

TEST_F(EtsInteropJsIntrinsTest, test_exception_forwarding)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("test_exception_forwarding_fromjs"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_exception_forwarding_fromets"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_exception_forwarding_recursive"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_core_error_forwarding"));
}

TEST_F(EtsInteropJsIntrinsTest, test_typechecks)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("test_typecheck_getprop"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_typecheck_jscall"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_typecheck_callets"));
}

TEST_F(EtsInteropJsIntrinsTest, test_accessor_throws)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("test_get_throws"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_set_throws"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_jscall_resolution_throws1"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_jscall_resolution_throws2"));
    ASSERT_EQ(true, CallEtsMethod<bool>("test_jscall_resolution_throws3"));
}

TEST_F(EtsInteropJsIntrinsTest, test_finalizers)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("test_finalizers"));
}

}  // namespace ark::ets::interop::js::testing
