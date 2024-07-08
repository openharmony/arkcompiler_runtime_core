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

class EtsConstructorTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConstructorTsToEtsTest, checkNamedClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkNamedClass"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateNamedClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateNamedClassFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkNamedClassInstance)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkNamedClassInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkAnonymousClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnonymousClass"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateAnonymousClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateAnonymousClassFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkAnonymousClassInstance)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnonymousClassInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkFunctionConstructor)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkFunctionConstructor"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateFunctionConstructorFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateFunctionConstructorFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkFunctionConstructorInstance)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkFunctionConstructorInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkIIFEClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIIFEClass"));
}
// NOTE issue(18074) fix IIFE without keyword 'new'.
TEST_F(EtsConstructorTsToEtsTest, DISABLED_checkIIFEClassError)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIIFEClassError"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateIIFEClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateIIFEClassFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkIIFEClassInstance)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIIFEClassInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkIIFEConstructor)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIIFEConstructor"));
}

TEST_F(EtsConstructorTsToEtsTest, checkIIFEConstructorUndefined)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIIFEConstructorUndefined"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateIIFEConstructorFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateIIFEConstructorFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkIIFEConstructorInstance)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkIIFEConstructorInstance"));
}
// NOTE issue(18077) fix try to call method with constructor.
TEST_F(EtsConstructorTsToEtsTest, DISABLED_checkMethodCreateConstructor)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodCreateConstructor"));
}
// NOTE issue(18077) fix try to call method with constructor.
TEST_F(EtsConstructorTsToEtsTest, DISABLED_checkCreateMethodConstructorClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateMethodConstructorClass"));
}
// NOTE issue(18077) fix try to call method with constructor.
TEST_F(EtsConstructorTsToEtsTest, DISABLED_checkMethodConstructorInstance)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodConstructorInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkMethodCreateAnonymousClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodCreateAnonymousClass"));
}

TEST_F(EtsConstructorTsToEtsTest, checkMethodCreateClassInstance)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkMethodCreateClassInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkSimpleObject)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSimpleObject"));
}
// NOTE issue(18072) fix create object with keyword new .
TEST_F(EtsConstructorTsToEtsTest, DISABLED_checkSimpleArrowFunction)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSimpleArrowFunction"));
}

TEST_F(EtsConstructorTsToEtsTest, checkAbstractClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAbstractClass"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateAbstractClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateAbstractClassFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkAbstractInstance)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAbstractInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkChildClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildClass"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateChildClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateChildClassFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkChildInstance)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkChildInstance"));
}

}  // namespace ark::ets::interop::js::testing
