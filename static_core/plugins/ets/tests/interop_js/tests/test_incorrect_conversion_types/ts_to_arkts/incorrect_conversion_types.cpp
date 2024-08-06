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

class EtsInteropIncorrectConversionTest : public EtsInteropTest {};

TEST_F(EtsInteropIncorrectConversionTest, conversionTsObjToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsObjToStr"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsArrayNumToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsArrayNumToStr"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsArrayStrToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsArrayStrToStr"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsArrayNumToNum)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsArrayNumToNum"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToNum)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsStrToNum"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsStrToInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToBool)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsStrToBool"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToByte)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsStrToByte"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToShort)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsStrToShort"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToLong)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsStrToLong"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToFloat)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsStrToFloat"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToDouble)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsStrToDouble"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToChar)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsStrToChar"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsStrToBigInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsStrToBigInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToNum)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBooleanToNum"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToByte)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBooleanToByte"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToShort)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBooleanToShort"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBooleanToInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToLong)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBooleanToLong"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToFloat)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBooleanToFloat"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToDouble)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBooleanToDouble"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToChar)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBooleanToChar"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBooleanToStr"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBooleanToBigInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBooleanToBigInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToNum)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsUndefinedToNum"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToByte)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsUndefinedToByte"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToShort)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsUndefinedToShort"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsUndefinedToInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToLong)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsUndefinedToLong"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToFloat)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsUndefinedToFloat"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToDouble)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsUndefinedToDouble"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToChar)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsUndefinedToChar"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToBool)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsUndefinedToBool"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsUndefinedToStr"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsUndefinedToBigInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsUndefinedToBigInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToNum)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNullToNum"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToByte)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNullToByte"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToShort)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNullToShort"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNullToInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToLong)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNullToLong"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToFloat)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNullToFloat"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToDouble)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNullToDouble"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToChar)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNullToChar"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToBool)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNullToBool"), true);
}
TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNumToInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToByte)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNumToByte"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToShort)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNumToShort"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToLong)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNumToLong"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToChar)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNumToChar"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToBool)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNumToBool"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNumToStr"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsNumToBigInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNumToBigInt"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsNullToBigInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsNullToBigInt"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToNum)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBigIntToNum"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToByte)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBigIntToByte"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToShort)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBigIntToShort"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBigIntToInt"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToLong)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBigIntToLong"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToFloat)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBigIntToFloat"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToDouble)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBigIntToDouble"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToChar)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBigIntToChar"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToBool)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBigIntToBool"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("conversionTsBigIntToStr"), true);
}

}  // namespace ark::ets::interop::js::testing
