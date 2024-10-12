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

class EtsAgeNameArgGenericTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsAgeNameArgGenericTsToEtsTest, checkArgFuFromSts)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkArgFuFromSts"));
}

TEST_F(EtsAgeNameArgGenericTsToEtsTest, checkTypeClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkTypeClass"));
}

TEST_F(EtsAgeNameArgGenericTsToEtsTest, checkClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkClassMethod"));
}

TEST_F(EtsAgeNameArgGenericTsToEtsTest, checkCreateClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassFromTs"));
}

TEST_F(EtsAgeNameArgGenericTsToEtsTest, checkCallMethodClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCallMethodClassFromTs"));
}

TEST_F(EtsAgeNameArgGenericTsToEtsTest, checkInstanceClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceClass"));
}

TEST_F(EtsAgeNameArgGenericTsToEtsTest, checkChildClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildClass"));
}

TEST_F(EtsAgeNameArgGenericTsToEtsTest, checkChildClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildClassMethod"));
}

TEST_F(EtsAgeNameArgGenericTsToEtsTest, checkCreateChildClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateChildClassFromTs"));
}

TEST_F(EtsAgeNameArgGenericTsToEtsTest, checkCallMethodChildClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCallMethodChildClassFromTs"));
}

TEST_F(EtsAgeNameArgGenericTsToEtsTest, checkInstanceChildClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceChildClass"));
}

}  // namespace ark::ets::interop::js::testing
