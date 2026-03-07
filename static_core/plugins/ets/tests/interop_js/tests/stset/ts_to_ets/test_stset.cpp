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

class TsToEtsSTSetTest : public EtsInteropTest {};

TEST_F(TsToEtsSTSetTest, testReturnSet)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testReturnSet"));
}

TEST_F(TsToEtsSTSetTest, testConsumeSet)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testConsumeSet"));
}

TEST_F(TsToEtsSTSetTest, testCheckUniqueness)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckUniqueness"));
}

TEST_F(TsToEtsSTSetTest, testCheckSizeIsProperty)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckSizeIsProperty"));
}

TEST_F(TsToEtsSTSetTest, testCheckInsertionOrder)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckInsertionOrder"));
}

TEST_F(TsToEtsSTSetTest, testCheckChaining)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckChaining"));
}

TEST_F(TsToEtsSTSetTest, testCheckNoIndexAccess)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckNoIndexAccess"));
}

TEST_F(TsToEtsSTSetTest, testCheckDeleteReturnsBoolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckDeleteReturnsBoolean"));
}

TEST_F(TsToEtsSTSetTest, testCheckHasMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckHasMethod"));
}

TEST_F(TsToEtsSTSetTest, testCheckClearMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCheckClearMethod"));
}

}  // namespace ark::ets::interop::js::testing
