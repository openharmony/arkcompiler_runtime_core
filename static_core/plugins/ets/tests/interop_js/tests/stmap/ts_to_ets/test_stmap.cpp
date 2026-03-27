/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

class TsToEtsSTMapTest : public EtsInteropTest {};

TEST_F(TsToEtsSTMapTest, testReturnMap)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testReturnMap"));
}

TEST_F(TsToEtsSTMapTest, testConsumeMap)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testConsumeMap"));
}

TEST_F(TsToEtsSTMapTest, testCheckDuplicateKeyOverwrites)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckDuplicateKeyOverwrites"));
}

TEST_F(TsToEtsSTMapTest, testCheckSizeIsProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckSizeIsProperty"));
}

TEST_F(TsToEtsSTMapTest, testCheckInsertionOrder)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckInsertionOrder"));
}

TEST_F(TsToEtsSTMapTest, testCheckChaining)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckChaining"));
}

TEST_F(TsToEtsSTMapTest, testCheckGetReturnsUndefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckGetReturnsUndefined"));
}

TEST_F(TsToEtsSTMapTest, testCheckDeleteReturnsBoolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckDeleteReturnsBoolean"));
}

TEST_F(TsToEtsSTMapTest, testCheckHasMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckHasMethod"));
}

TEST_F(TsToEtsSTMapTest, testCheckClearMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckClearMethod"));
}

TEST_F(TsToEtsSTMapTest, testCheckForEachMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckForEachMethod"));
}

TEST_F(TsToEtsSTMapTest, testCheckEntriesMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckEntriesMethod"));
}

TEST_F(TsToEtsSTMapTest, testCheckKeysMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckKeysMethod"));
}

TEST_F(TsToEtsSTMapTest, testCheckValuesMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckValuesMethod"));
}

TEST_F(TsToEtsSTMapTest, testInstanceOfMap)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInstanceOfMap"));
}

TEST_F(TsToEtsSTMapTest, test_stMap)
{
    ASSERT_TRUE(RunJsTestSuite("stmap.ts"));
}

}  // namespace ark::ets::interop::js::testing
