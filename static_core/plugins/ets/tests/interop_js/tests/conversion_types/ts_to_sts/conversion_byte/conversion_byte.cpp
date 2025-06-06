/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

class EtsConversionByteTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConversionByteTsToEtsTest, checkConversionByteToInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToInt"));
}

TEST_F(EtsConversionByteTsToEtsTest, checkConversionByteToNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToNumber"));
}

TEST_F(EtsConversionByteTsToEtsTest, checkConversionByteToFloat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToFloat"));
}

TEST_F(EtsConversionByteTsToEtsTest, checkConversionByteToByte)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToByte"));
}

TEST_F(EtsConversionByteTsToEtsTest, checkConversionByteToShort)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToShort"));
}

TEST_F(EtsConversionByteTsToEtsTest, checkConversionByteToLong)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToLong"));
}

TEST_F(EtsConversionByteTsToEtsTest, checkConversionByteToDouble)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToDouble"));
}

TEST_F(EtsConversionByteTsToEtsTest, checkConversionByteToChar)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToChar"));
}

}  // namespace ark::ets::interop::js::testing
