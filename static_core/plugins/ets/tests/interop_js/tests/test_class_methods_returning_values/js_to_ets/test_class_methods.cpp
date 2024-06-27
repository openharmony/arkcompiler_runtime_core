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

class EtsInteropClassMethodsReturningValuesTest : public EtsInteropTest {};

// Note: Several tests disabled until #17741 is resolved

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnIntegerAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnIntegerAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnStringAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnStringAsAny");
    ASSERT_EQ(ret, true);
}

// Note: Test is disabled until #17741 is resolved
TEST_F(EtsInteropClassMethodsReturningValuesTest, DISABLED_eTS_call_TS_TestReturnBigIntegerAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnBigIntegerAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnBooleanAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnBooleanAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnUndefinedAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnUndefinedAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnNullAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnNullAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnMapAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnMapAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnSetAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnSetAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnStringAsLiteral)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnStringAsLiteral");
    ASSERT_EQ(ret, true);
}

// Note: Test is disabled until #17741 is resolved
TEST_F(EtsInteropClassMethodsReturningValuesTest, DISABLED_eTS_call_TS_TestReturnBigNAsLiteral)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnBigNAsLiteral");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnIntAsLiteral)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnIntAsLiteral");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnBoolAsLiteral)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnBoolAsLiteral");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnMap)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnMap");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnSet)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnSet");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnTuple)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnTuple");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnStringSubsetByRef)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnStringSubsetByRef");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnMapSubsetByRef)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnMapSubsetByRef");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnStringSubsetByValue)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnStringSubsetByValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnIntSubsetByValue)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnIntSubsetByValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnLongIntSubsetByValue)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnLongIntSubsetByValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnBoolSubsetByValue)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnBoolSubsetByValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnUndefinedSubsetByValue)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnUndefinedSubsetByValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnNullSubsetByValue)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnNullSubsetByValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnUnion)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnUnion");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnClass)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnClass");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnInterface)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsMethod<bool>("TestReturnInterface");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
