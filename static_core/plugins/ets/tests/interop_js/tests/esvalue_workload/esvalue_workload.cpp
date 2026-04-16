/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

class ESValueWEorkloadTest : public EtsInteropTest {};

TEST_F(ESValueWEorkloadTest, testGetPropertyString50)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetPropertyString50"));
}

TEST_F(ESValueWEorkloadTest, testGetPropertyString100)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetPropertyString100"));
}

TEST_F(ESValueWEorkloadTest, testGetPropertyString500)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetPropertyString500"));
}

TEST_F(ESValueWEorkloadTest, testGetPropertyString1000)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetPropertyString1000"));
}

TEST_F(ESValueWEorkloadTest, testGetPropertyString5000)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetPropertyString5000"));
}

TEST_F(ESValueWEorkloadTest, testGetPropertyString10000)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetPropertyString10000"));
}

TEST_F(ESValueWEorkloadTest, testGetPropertyString50000)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetPropertyString50000"));
}

TEST_F(ESValueWEorkloadTest, testGetPropertyBoolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetPropertyBoolean"));
}

TEST_F(ESValueWEorkloadTest, testGetPropertyNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetPropertyNumber"));
}

TEST_F(ESValueWEorkloadTest, testGetPropertyObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testGetPropertyObject"));
}

TEST_F(ESValueWEorkloadTest, testSetPropertyString50)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetPropertyString50"));
}

TEST_F(ESValueWEorkloadTest, testSetPropertyString100)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetPropertyString100"));
}

TEST_F(ESValueWEorkloadTest, testSetPropertyString500)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetPropertyString500"));
}

TEST_F(ESValueWEorkloadTest, testSetPropertyString1000)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetPropertyString1000"));
}

TEST_F(ESValueWEorkloadTest, testSetPropertyString5000)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetPropertyString5000"));
}

TEST_F(ESValueWEorkloadTest, testSetPropertyString10000)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetPropertyString10000"));
}

TEST_F(ESValueWEorkloadTest, testSetPropertyString50000)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetPropertyString50000"));
}

TEST_F(ESValueWEorkloadTest, testSetPropertyBoolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetPropertyBoolean"));
}

TEST_F(ESValueWEorkloadTest, testSetPropertyNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetPropertyNumber"));
}

TEST_F(ESValueWEorkloadTest, testSetPropertyObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSetPropertyObject"));
}

TEST_F(ESValueWEorkloadTest, testInvokeNoArgs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeNoArgs"));
}

TEST_F(ESValueWEorkloadTest, testInvokeOneArgNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeOneArgNumber"));
}

TEST_F(ESValueWEorkloadTest, testInvokeOneArgString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeOneArgString"));
}

TEST_F(ESValueWEorkloadTest, testInvokeOneArgObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeOneArgObject"));
}

TEST_F(ESValueWEorkloadTest, testInvokeFiveArgsNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeFiveArgsNumber"));
}

TEST_F(ESValueWEorkloadTest, testInvokeFiveArgsString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeFiveArgsString"));
}

TEST_F(ESValueWEorkloadTest, testInvokeFiveArgsObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeFiveArgsObject"));
}

TEST_F(ESValueWEorkloadTest, testInvokeMethodNoArgs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeMethodNoArgs"));
}

TEST_F(ESValueWEorkloadTest, testInvokeMethodOneArgNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeMethodOneArgNumber"));
}

TEST_F(ESValueWEorkloadTest, testInvokeMethodOneArgString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeMethodOneArgString"));
}

TEST_F(ESValueWEorkloadTest, testInvokeMethodOneArgObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeMethodOneArgObject"));
}

TEST_F(ESValueWEorkloadTest, testInvokeMethodFiveArgsNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeMethodFiveArgsNumber"));
}

TEST_F(ESValueWEorkloadTest, testInvokeMethodFiveArgsString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeMethodFiveArgsString"));
}

TEST_F(ESValueWEorkloadTest, testInvokeMethodFiveArgsObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testInvokeMethodFiveArgsObject"));
}

}  // namespace ark::ets::interop::js::testing
