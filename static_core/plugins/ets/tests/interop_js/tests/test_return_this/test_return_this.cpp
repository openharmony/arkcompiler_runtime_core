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
#include "utils/logger.h"

namespace ark::ets::interop::js::testing {

class EtsInteropReturnThisTest : public EtsInteropTest {};

TEST_F(EtsInteropReturnThisTest, TestReturnThisInClass)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("testReturnThisInClass"), true);
}

TEST_F(EtsInteropReturnThisTest, TestReturnThisAsNumber)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("testReturnThisAsNumber"), true);
}

TEST_F(EtsInteropReturnThisTest, TestReturnThisAsString)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("testReturnThisAsString"), true);
}

TEST_F(EtsInteropReturnThisTest, TestReturnThisAsNull)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("testReturnThisAsNull"), true);
}

// Note: Test is disabled until #17852 is resolved
TEST_F(EtsInteropReturnThisTest, DISABLED_eTS_call_TS_TestReturnThisAsMap)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("testReturnThisAsMap"), true);
}

// Note: Test is disabled until #18365 is resolved
TEST_F(EtsInteropReturnThisTest, DISABLED_eTS_call_TS_TestReturnThisAsTuple)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("testReturnThisAsTuple"), true);
}

// Note: Test is disabled until #18365 is resolved
TEST_F(EtsInteropReturnThisTest, DISABLED_eTS_call_TS_TestReturnThisAsArray)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("testReturnThisAsArray"), true);
}

TEST_F(EtsInteropReturnThisTest, TestReturnThisAsObj)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("testReturnThisAsObj"), true);
}

TEST_F(EtsInteropReturnThisTest, TestReturnThisAsObjClass)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("testReturnThisAsObjClass"), true);
}

TEST_F(EtsInteropReturnThisTest, TestReturnThisAsFunc)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("testReturnThisAsFunc"), true);
}

// Note: Test is disabled until #18468 is resolved
TEST_F(EtsInteropReturnThisTest, DISABLED_eTS_call_TS_TestReturnThisAsAsyncFunc)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsMethod<bool>("testReturnThisAsAsyncFunc"), true);
}

}  // namespace ark::ets::interop::js::testing
