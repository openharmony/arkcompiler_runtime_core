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

class EtsConversionBigIntTsToEtsTest : public EtsInteropTest {};

// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkTypeBigInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkTypeBigInt"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToInt"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToNumber"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToFloat"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToByte"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToShort"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToLong"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToDouble"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToChar)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToChar"));
}

}  // namespace ark::ets::interop::js::testing
