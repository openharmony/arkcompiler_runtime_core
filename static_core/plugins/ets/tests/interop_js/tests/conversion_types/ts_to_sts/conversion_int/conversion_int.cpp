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

class EtsConversionIntTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConversionIntTsToEtsTest, checkConversionToInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionToInt"));
}

TEST_F(EtsConversionIntTsToEtsTest, checkConversionToNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionToNumber"));
}

TEST_F(EtsConversionIntTsToEtsTest, checkConversionToFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionToFloat"));
}

TEST_F(EtsConversionIntTsToEtsTest, checkConversionToByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionToByte"));
}

TEST_F(EtsConversionIntTsToEtsTest, checkConversionToShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionToShort"));
}

TEST_F(EtsConversionIntTsToEtsTest, checkConversionToLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionToLong"));
}

TEST_F(EtsConversionIntTsToEtsTest, checkConversionToDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionToDouble"));
}

TEST_F(EtsConversionIntTsToEtsTest, checkConversionToChar)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionToChar"));
}

}  // namespace ark::ets::interop::js::testing
