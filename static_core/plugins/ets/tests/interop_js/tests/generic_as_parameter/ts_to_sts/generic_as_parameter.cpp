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

class EtsGenericAsParameterTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterInt"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterString)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterBool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterArr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterArr"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterObj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterUnion)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterUnion"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterTuple)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterTuple"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterExplicitCallInt"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallString)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterExplicitCallString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallBool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterExplicitCallBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallArr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterExplicitCallArr"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallObj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterExplicitCallObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallUnion)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterExplicitCallUnion"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallTuple)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterExplicitCallTuple"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallLiteral)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkAnyTypeParameterExplicitCallLiteral"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionReturnAnyInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionReturnAnyInt"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionReturnAnyString)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionReturnAnyString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionReturnAnyBool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionReturnAnyBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionReturnAnyArr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionReturnAnyArr"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionReturnAnyObj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionReturnAnyObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionReturnAnyUnion)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionReturnAnyUnion"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionReturnAnyTuple)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionReturnAnyTuple"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionExplicitCallInt"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallString)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionExplicitCallString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallBool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionExplicitCallBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallArr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionExplicitCallArr"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallObj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionExplicitCallObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallUnion)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionExplicitCallUnion"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallTuple)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionExplicitCallTuple"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallLiteral)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericTypeFunctionExplicitCallLiteral"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericNumber)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkExtendGenericNumber"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericString)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkExtendGenericString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericBool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkExtendGenericBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericArr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkExtendGenericArr"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericObj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkExtendGenericObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericUnion)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkExtendGenericUnion"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericTuple)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkExtendGenericTuple"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericLiteral)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkExtendGenericLiteral"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericExtendInterface)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericExtendInterface"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericExtendType)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericExtendType"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkTupleGeneric)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkTupleGeneric"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkCollectGenericInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCollectGenericInt"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkCollectGenericString)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCollectGenericString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkCollectGenericBool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCollectGenericBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkCollectGenericObj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCollectGenericObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericUserClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericUserClass"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkCreateClassFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkCreateClassFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkUserClassInstance)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkUserClassInstance"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultInt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultInt"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultString)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultBool)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultArr)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultArr"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultObj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultUnion)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultUnion"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultTuple)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultTuple"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultLiteral)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultLiteral"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultIntCallFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultIntCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultStringCallFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultStringCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultBoolCallFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultBoolCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultArrCallFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultArrCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultObjCallFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultObjCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultUnionCallFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultUnionCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultTupleCallFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultTupleCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultLiteralCallFromTs)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericDefaultLiteralCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericExtendClass)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("checkGenericExtendClass"));
}

}  // namespace ark::ets::interop::js::testing
