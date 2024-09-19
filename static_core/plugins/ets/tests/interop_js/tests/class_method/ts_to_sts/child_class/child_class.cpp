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

class EtsClassMethodChildClassTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsClassMethodChildClassTsToEtsTest, checkChildClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildClassMethod"));
}

TEST_F(EtsClassMethodChildClassTsToEtsTest, checkCreateChildClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateChildClassFromTs"));
}

TEST_F(EtsClassMethodChildClassTsToEtsTest, checkInstanceChildClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceChildClass"));
}

TEST_F(EtsClassMethodChildClassTsToEtsTest, checkChildClassMethodOwnMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildClassMethodOwnMethod"));
}

TEST_F(EtsClassMethodChildClassTsToEtsTest, checkCreateChildClassFromTsOwnMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateChildClassFromTsOwnMethod"));
}

TEST_F(EtsClassMethodChildClassTsToEtsTest, checkInstanceChildClassOwnMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceChildClassOwnMethod"));
}

}  // namespace ark::ets::interop::js::testing
