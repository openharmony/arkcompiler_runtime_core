/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

class ArrayTsToEtsTest : public EtsInteropTest {};

TEST_F(ArrayTsToEtsTest, indexAccess)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("indexAccess"));
}

TEST_F(ArrayTsToEtsTest, testLength)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testLength"));
}

TEST_F(ArrayTsToEtsTest, testArrayTypeof)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("typeofArray"));
}

TEST_F(ArrayTsToEtsTest, testInstanceof)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("instanceofArray"));
}

TEST_F(ArrayTsToEtsTest, testInstanceofObj)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("instanceofArrayObject"));
}

TEST_F(ArrayTsToEtsTest, testAt)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testAt"));
}

TEST_F(ArrayTsToEtsTest, testConcat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testConcat"));
}

TEST_F(ArrayTsToEtsTest, testCopyWithin)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testCopyWithin"));
}

TEST_F(ArrayTsToEtsTest, testEvery)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testEvery"));
}

TEST_F(ArrayTsToEtsTest, testFill)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testFill"));
}

TEST_F(ArrayTsToEtsTest, testFilter)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testFilter"));
}

TEST_F(ArrayTsToEtsTest, testFind)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testFind"));
}

TEST_F(ArrayTsToEtsTest, testFindIndex)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testFindIndex"));
}

TEST_F(ArrayTsToEtsTest, testFindLast)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testFindLast"));
}

TEST_F(ArrayTsToEtsTest, testFindLastIndex)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testFindLastIndex"));
}

TEST_F(ArrayTsToEtsTest, testFlat)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testFlat"));
}

TEST_F(ArrayTsToEtsTest, testFlatMap)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testFlatMap"));
}

TEST_F(ArrayTsToEtsTest, testForEach)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testForEach"));
}

TEST_F(ArrayTsToEtsTest, testIncludes)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testIncludes"));
}

TEST_F(ArrayTsToEtsTest, testIndexOf)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testIndexOf"));
}

TEST_F(ArrayTsToEtsTest, testJoin)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testJoin"));
}

TEST_F(ArrayTsToEtsTest, testLastIndexOf)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testLastIndexOf"));
}

TEST_F(ArrayTsToEtsTest, testMap)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testMap"));
}

TEST_F(ArrayTsToEtsTest, testPop)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testPop"));
}

TEST_F(ArrayTsToEtsTest, testPush)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testPush"));
}

TEST_F(ArrayTsToEtsTest, testReduce)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testReduce"));
}

TEST_F(ArrayTsToEtsTest, testReduceRight)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testReduceRight"));
}

TEST_F(ArrayTsToEtsTest, testReverse)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testReverse"));
}

TEST_F(ArrayTsToEtsTest, testShift)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testShift"));
}

TEST_F(ArrayTsToEtsTest, testSlice)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testSlice"));
}

TEST_F(ArrayTsToEtsTest, testSome)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testSome"));
}

TEST_F(ArrayTsToEtsTest, testSort)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testSort"));
}

TEST_F(ArrayTsToEtsTest, testSplice)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testSplice"));
}

TEST_F(ArrayTsToEtsTest, testToLocaleString)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testToLocaleString"));
}

TEST_F(ArrayTsToEtsTest, testToReversed)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testToReversed"));
}

TEST_F(ArrayTsToEtsTest, testToSorted)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testToSorted"));
}

TEST_F(ArrayTsToEtsTest, testToSpliced)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testToSpliced"));
}

TEST_F(ArrayTsToEtsTest, testUnShift)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testUnShift"));
}

TEST_F(ArrayTsToEtsTest, testWith)
{
    ASSERT_EQ(true, CallEtsMethod<bool>("testWith"));
}

}  // namespace ark::ets::interop::js::testing