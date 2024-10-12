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

class EtsIntersectionArgInterfaceTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkArgFuFromSts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkArgFuFromSts"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkAgeNameClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAgeNameClass"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkAgeNameClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAgeNameClassMethod"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkCreateAgeNameClassInterfaceFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateAgeNameClassInterfaceFromTs"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkCallMethodAgeNameClassInterfaceFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCallMethodAgeNameClassInterfaceFromTs"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkInstanceAgeNameInterfaceClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceAgeNameInterfaceClass"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkChildAgeNameInterfaceClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildAgeNameInterfaceClass"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkChildAgeNameInterfaceClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildAgeNameInterfaceClassMethod"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkCreateChildAgeNameClassInterfaceFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateChildAgeNameClassInterfaceFromTs"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkCallMethodChildAgeNameClassInterfaceFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCallMethodChildAgeNameClassInterfaceFromTs"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkInstanceChildAgeNameInterfaceClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceChildAgeNameInterfaceClass"));
}

}  // namespace ark::ets::interop::js::testing
