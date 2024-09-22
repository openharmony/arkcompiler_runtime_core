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

class EtsConversionArrayIntTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayChar)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntToArrayChar"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayChar)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToArrayChar"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntStringToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntStringToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntStringToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntStringToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntStringToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntStringToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntStringToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayChar)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionIntStringToArrayChar"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayChar)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToArrayChar"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionFloatToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionFloatToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionFloatToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionFloatToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionFloatToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionFloatToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionFloatToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayChar)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionFloatToArrayChar"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionByteToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionByteToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionByteToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionByteToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionByteToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionByteToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionByteToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayChar)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionByteToArrayChar"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToArrayInt"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToArrayNumber"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToArrayFloat"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToArrayByte"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToArrayShort"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToArrayLong"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntArrayDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToArrayDouble"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayChar)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBigIntToArrayChar"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialArrayDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayChar)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToArrayChar"));
}

}  // namespace ark::ets::interop::js::testing
