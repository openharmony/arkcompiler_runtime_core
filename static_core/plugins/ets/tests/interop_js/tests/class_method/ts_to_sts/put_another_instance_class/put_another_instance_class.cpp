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

class EtsClassMethodPutAnotherInstanceClassTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkChildClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateChildClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateChildClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceChildClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceChildClass"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkChildClassMethodOwnMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildClassMethodOwnMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateChildClassFromTsOwnMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateChildClassFromTsOwnMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceChildClassOwnMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceChildClassOwnMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkUserClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkUserClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateUserClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateUserClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceUserClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceUserClass"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInterfaceClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInterfaceClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateInterfaceClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateInterfaceClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceInterfaceClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceInterfaceClass"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkStaticClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkStaticClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateStaticClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateStaticClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkPrivateClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkPrivateClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreatePrivateClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreatePrivateClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstancePrivateClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstancePrivateClass"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkChildProtectedClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildProtectedClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateChildProtectedClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateChildProtectedClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceChildProtectedClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceChildProtectedClass"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkProtectedClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkProtectedClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateProtectedClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateProtectedClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceProtectedClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceProtectedClass"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkAbstractClassMethod)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAbstractClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateAbstractClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateAbstractClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceAbstractClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkInstanceAbstractClass"));
}

}  // namespace ark::ets::interop::js::testing
