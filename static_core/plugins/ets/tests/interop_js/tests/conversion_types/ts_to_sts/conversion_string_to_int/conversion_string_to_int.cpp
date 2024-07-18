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

class EtsConversionStringToIntTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConversionStringToIntTsToEtsTest, checkConversionStringToInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionStringToInt"));
}

TEST_F(EtsConversionStringToIntTsToEtsTest, checkConversionStringToNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionStringToNumber"));
}

TEST_F(EtsConversionStringToIntTsToEtsTest, checkConversionStringToFloat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionStringToFloat"));
}

TEST_F(EtsConversionStringToIntTsToEtsTest, checkConversionStringToByte)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionStringToByte"));
}

TEST_F(EtsConversionStringToIntTsToEtsTest, checkConversionStringToShort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionStringToShort"));
}

TEST_F(EtsConversionStringToIntTsToEtsTest, checkConversionStringToLong)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionStringToLong"));
}

TEST_F(EtsConversionStringToIntTsToEtsTest, checkConversionStringToDouble)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkConversionStringToDouble"));
}

}  // namespace ark::ets::interop::js::testing
