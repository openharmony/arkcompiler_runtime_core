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

class EtsConversionHexadecimalTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConversionHexadecimalTsToEtsTest, checkConversionHexadecimalToInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToInt"));
}

TEST_F(EtsConversionHexadecimalTsToEtsTest, checkConversionHexadecimalToNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToNumber"));
}

TEST_F(EtsConversionHexadecimalTsToEtsTest, checkConversionHexadecimalToFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToFloat"));
}

TEST_F(EtsConversionHexadecimalTsToEtsTest, checkConversionHexadecimalToByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToByte"));
}

TEST_F(EtsConversionHexadecimalTsToEtsTest, checkConversionHexadecimalToShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToShort"));
}

TEST_F(EtsConversionHexadecimalTsToEtsTest, checkConversionHexadecimalToLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToLong"));
}

TEST_F(EtsConversionHexadecimalTsToEtsTest, checkConversionHexadecimalToDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToDouble"));
}

TEST_F(EtsConversionHexadecimalTsToEtsTest, checkConversionHexadecimalToChar)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionHexadecimalToChar"));
}

}  // namespace ark::ets::interop::js::testing
