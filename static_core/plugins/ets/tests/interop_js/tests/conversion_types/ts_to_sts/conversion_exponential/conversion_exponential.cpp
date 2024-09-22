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

class EtsConversionExponentialTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConversionExponentialTsToEtsTest, checkConversionExponentialToInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToInt"));
}

TEST_F(EtsConversionExponentialTsToEtsTest, checkConversionExponentialToNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToNumber"));
}

TEST_F(EtsConversionExponentialTsToEtsTest, checkConversionExponentialToFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToFloat"));
}

TEST_F(EtsConversionExponentialTsToEtsTest, checkConversionExponentialToByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToByte"));
}

TEST_F(EtsConversionExponentialTsToEtsTest, checkConversionExponentialToShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToShort"));
}

TEST_F(EtsConversionExponentialTsToEtsTest, checkConversionExponentialToLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToLong"));
}

TEST_F(EtsConversionExponentialTsToEtsTest, checkConversionExponentialToDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToDouble"));
}

TEST_F(EtsConversionExponentialTsToEtsTest, checkConversionExponentialToChar)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionExponentialToChar"));
}

}  // namespace ark::ets::interop::js::testing
