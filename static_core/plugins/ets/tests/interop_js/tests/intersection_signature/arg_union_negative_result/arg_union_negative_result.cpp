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

class EtsIntersectionArgUnionNegativeResultTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsIntersectionArgUnionNegativeResultTsToEtsTest, checkArgFuFromSts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkArgFuFromSts"));
}

TEST_F(EtsIntersectionArgUnionNegativeResultTsToEtsTest, checkIntersectionClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIntersectionClass"));
}

TEST_F(EtsIntersectionArgUnionNegativeResultTsToEtsTest, checkIntersectionClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIntersectionClassMethod"));
}

TEST_F(EtsIntersectionArgUnionNegativeResultTsToEtsTest, checkInstanceIntersectionTypeClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceIntersectionTypeClass"));
}

TEST_F(EtsIntersectionArgUnionNegativeResultTsToEtsTest, checkChildIntersectionClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildIntersectionClass"));
}

TEST_F(EtsIntersectionArgUnionNegativeResultTsToEtsTest, checkChildIntersectionClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildIntersectionClassMethod"));
}

TEST_F(EtsIntersectionArgUnionNegativeResultTsToEtsTest, checkInstanceChildIntersectionTypeClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceChildIntersectionTypeClass"));
}

}  // namespace ark::ets::interop::js::testing
