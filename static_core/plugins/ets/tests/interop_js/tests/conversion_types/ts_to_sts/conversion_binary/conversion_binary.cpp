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

class EtsConversionBinaryTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToInt"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToNumber"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToFloat"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToByte"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToShort"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToLong"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToDouble"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToChar)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionBinaryToChar"));
}

}  // namespace ark::ets::interop::js::testing
