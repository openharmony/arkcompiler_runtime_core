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

class EtsIntersectionArgAgeNameNegativeResultTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkArgFuFromSts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkArgFuFromSts"));
}

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkAgeNameClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAgeNameClass"));
}

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkAgeNameClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAgeNameClassMethod"));
}

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkInstanceAgeNameInterfaceClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceAgeNameInterfaceClass"));
}

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkChildAgeNameClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildAgeNameClass"));
}

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkChildAgeNameClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildAgeNameClassMethod"));
}

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkInstanceChildAgeNameInterfaceClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceChildAgeNameInterfaceClass"));
}

}  // namespace ark::ets::interop::js::testing
