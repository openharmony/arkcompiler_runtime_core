/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class EtsThreadScopeTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsThreadScopeTsToEtsTest, TestGetObjectPropertis)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testGetObjectPropertis"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestSetObjectPropertis)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testSetObjectPropertis"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestGetStringPropertis)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testGetStringPropertis"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestSetStringPropertis)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testSetStringPropertis"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestGetStringPropertis50)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testGetStringPropertis50"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestSetStringPropertis50)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testSetStringPropertis50"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestGetStringPropertis100)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testGetStringPropertis100"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestSetStringPropertis100)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testSetStringPropertis100"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestGetStringPropertis500)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testGetStringPropertis500"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestSetStringPropertis500)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testSetStringPropertis500"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestGetStringPropertis1000)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testGetStringPropertis1000"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestSetStringPropertis1000)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testSetStringPropertis1000"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestGetStringPropertis5000)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testGetStringPropertis5000"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestSetStringPropertis5000)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testSetStringPropertis5000"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestGetStringPropertis10000)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testGetStringPropertis10000"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestSetStringPropertis10000)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testSetStringPropertis10000"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestGetStringPropertis50000)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testGetStringPropertis50000"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestSetStringPropertis50000)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testSetStringPropertis50000"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestSetNumberPropertis)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testGetNumberPropertis"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestGetNumberPropertis)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testSetNumberPropertis"), true);
}

TEST_F(EtsThreadScopeTsToEtsTest, TestCallFooFunc)
{
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "testCallFooFunc"), true);
}

}  // namespace ark::ets::interop::js::testing