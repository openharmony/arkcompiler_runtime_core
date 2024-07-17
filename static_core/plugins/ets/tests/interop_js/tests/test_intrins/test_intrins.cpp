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
        LoadModuleAs("test_intrins", "index.js");
    }
};

TEST_F(EtsInteropJsIntrinsTest, basic1)
{
    constexpr double ARG0 = 271;
    constexpr double ARG1 = 314;
    constexpr double RES = ARG0 + ARG1;
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<double>("jsSumWrapperNumNum", ARG0, ARG1);
    ASSERT_EQ(ret, RES);
}

TEST_F(EtsInteropJsIntrinsTest, basic2)
{
    constexpr double ARG0 = 12.34;
    constexpr std::string_view ARG1 = "foo";
    constexpr std::string_view RES = "12.34foo";
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<std::string>("jsSumWrapperNumStr", ARG0, ARG1);
    ASSERT_EQ(ret, RES);
}

TEST_F(EtsInteropJsIntrinsTest, test_convertors)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testUndefined"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testNull"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testBoolean"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testNumber"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testString"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testObject"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testBigint"));
    // NOTE(vpukhov): symbol, function, external

    ASSERT_EQ(true, CallEtsMethod<bool>("testStringOps"));
}

TEST_F(EtsInteropJsIntrinsTest, test_builtin_array_convertors)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testBuiltinArrayAny"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testBuiltinArrayBoolean"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testBuiltinArrayInt"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testBuiltinArrayNumber"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testBuiltinArrayString"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testBuiltinArrayObject"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testBuiltinArrayMultidim"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testBuiltinArrayInstanceof"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testInitArrayComponent"));
}

TEST_F(EtsInteropJsIntrinsTest, test_named_access)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testNamedAccess"));
}

TEST_F(EtsInteropJsIntrinsTest, test_newcall)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testNewcall"));
}

TEST_F(EtsInteropJsIntrinsTest, test_value_call)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testValueCall"));
}

TEST_F(EtsInteropJsIntrinsTest, test_call_stress)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testCallStress"));
}

// Remove after JSValue cast fix
TEST_F(EtsInteropJsIntrinsTest, DISABLED_test_undefined_cast_bug)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testUndefinedCastBug"));
}

TEST_F(EtsInteropJsIntrinsTest, test_lambda_proxy)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testLambdaProxy"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testLambdaProxyRecursive"));
}

TEST_F(EtsInteropJsIntrinsTest, test_exception_forwarding)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testExceptionForwardingFromjs"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testExceptionForwardingFromets"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testExceptionForwardingRecursive"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testCoreErrorForwarding"));
}

TEST_F(EtsInteropJsIntrinsTest, test_typechecks)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testTypecheckGetProp"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testTypecheckJscall"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testTypecheckCallets"));
}

TEST_F(EtsInteropJsIntrinsTest, test_accessor_throws)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testGetThrows"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testSetThrows"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testJscallResolutionThrows1"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testJscallResolutionThrows2"));
    ASSERT_EQ(true, CallEtsMethod<bool>("testJscallResolutionThrows3"));
}

TEST_F(EtsInteropJsIntrinsTest, test_finalizers)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testFinalizers"));
}

}  // namespace ark::ets::interop::js::testing
