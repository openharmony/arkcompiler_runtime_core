/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

class InvalidOpESValTsToEtsTest : public EtsInteropTest {};

TEST_F(InvalidOpESValTsToEtsTest, checkUndefinedGetProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUndefinedGetProperty"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkUndefinedHasProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUndefinedHasProperty"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkUndefinedToNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUndefinedToNumber"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkUndefinedToString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUndefinedToString"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkUndefinedSetIndex)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUndefinedSetIndex"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkUndefinedInvokeMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUndefinedInvokeMethod"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkUndefinedInstantiate)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUndefinedInstantiate"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkNullSetProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkNullSetProperty"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkNullHasOwnProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkNullHasOwnProperty"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkNullToString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkNullToString"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkNullGetIndex)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkNullGetIndex"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkNullInvoke)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkNullInvoke"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkIntGetProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIntGetProperty"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkIntToString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIntToString"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkIntInvoke)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIntInvoke"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkIntHasProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIntHasProperty"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkBooleanInvokeMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkBooleanInvokeMethod"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkBigintInstantiate)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkBigintInstantiate"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkStringSetProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkStringSetProperty"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkIntSetIndex)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIntSetIndex"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkTsGetProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTsGetProperty"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkTsInstantiate)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTsInstantiate"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkTsInvoke)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTsInvoke"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkTsGetIndex)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTsGetIndex"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkTsHasProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTsHasProperty"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkTsToBoolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTsToBoolean"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkTsToPromise)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTsToPromise"));
}

TEST_F(InvalidOpESValTsToEtsTest, checkTsToString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTsToString"));
}

}  // namespace ark::ets::interop::js::testing