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

class EtsStaticMethodTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsStaticMethodTsToEtsTest, checkGenericStaticInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericStaticInt"));
}

TEST_F(EtsStaticMethodTsToEtsTest, checkGenericStaticString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericStaticString"));
}

TEST_F(EtsStaticMethodTsToEtsTest, checkGenericStaticBool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericStaticBool"));
}

TEST_F(EtsStaticMethodTsToEtsTest, checkGenericStaticArr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericStaticArr"));
}

TEST_F(EtsStaticMethodTsToEtsTest, checkGenericStaticMethodCallFromTsInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericStaticMethodCallFromTsInt"));
}

TEST_F(EtsStaticMethodTsToEtsTest, checkGenericStaticMethodCallFromTsString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericStaticMethodCallFromTsString"));
}

TEST_F(EtsStaticMethodTsToEtsTest, checkGenericStaticMethodCallFromTsBool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericStaticMethodCallFromTsBool"));
}

TEST_F(EtsStaticMethodTsToEtsTest, checkGenericStaticMethodCallFromTsArr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericStaticMethodCallFromTsArr"));
}

}  // namespace ark::ets::interop::js::testing
