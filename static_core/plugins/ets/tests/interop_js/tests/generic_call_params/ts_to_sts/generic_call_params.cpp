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

class EtsGenericCallParamsTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsGenericCallParamsTsToEtsTest, checkApplyFunctionGenericString)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkApplyFunctionGenericString"));
}

TEST_F(EtsGenericCallParamsTsToEtsTest, checkApplyFunctionGenericInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkApplyFunctionGenericInt"));
}

TEST_F(EtsGenericCallParamsTsToEtsTest, checkApplyFunctionGenericTuple)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkApplyFunctionGenericTuple"));
}

TEST_F(EtsGenericCallParamsTsToEtsTest, checkGenericExtendsClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericExtendsClass"));
}

TEST_F(EtsGenericCallParamsTsToEtsTest, checkGenericExtendsClassCallFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericExtendsClassCallFromTs"));
}

TEST_F(EtsGenericCallParamsTsToEtsTest, checkApplyFunctionGenericArrayCallFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkApplyFunctionGenericArrayCallFromTs"));
}

TEST_F(EtsGenericCallParamsTsToEtsTest, checkApplyFunctionGenericArray)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkApplyFunctionGenericArray"));
}
// NOTE: issue (18005) fix param keyword function
TEST_F(EtsGenericCallParamsTsToEtsTest, DISABLED_CheckApplyFunWithConstraintsFun)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkApplyFunWithConstraintsFun"));
}

TEST_F(EtsGenericCallParamsTsToEtsTest, checkApplyFunWithConstraintsArrowFun)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkApplyFunWithConstraintsArrowFun"));
}

TEST_F(EtsGenericCallParamsTsToEtsTest, checkGenericKeyof)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericKeyof"));
}
// NOTE: issue (18015) fix function as param
TEST_F(EtsGenericCallParamsTsToEtsTest, DISABLED_CheckClassSubset)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkClassSubset"));
}

TEST_F(EtsGenericCallParamsTsToEtsTest, checkSubsetClassCallFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkSubsetClassCallFromTs"));
}

TEST_F(EtsGenericCallParamsTsToEtsTest, checkApplyFunctionGenericUnion)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkApplyFunctionGenericUnion"));
}

}  // namespace ark::ets::interop::js::testing
