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

class EtsTestErrorTSToETSTest : public EtsInteropTest {};

TEST_F(EtsTestErrorTSToETSTest, testCatchRangeError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchRangeError"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchReferenceError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchReferenceError"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchSyntaxError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchSyntaxError"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchURIError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchURIError"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchTypeError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchTypeError"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchEvalError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchEvalError"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchAggregateError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchAggregateError"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchExtendRangeError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchExtendRangeError"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchExtendReferenceError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchExtendReferenceError"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchExtendSyntaxError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchExtendSyntaxError"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchExtendURIError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchExtendURIError"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchExtendTypeError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchExtendTypeError"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchNonError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchNonError"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchErrorSubClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchErrorSubClass"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchErrorWithObjectMessage)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchErrorWithObjectMessage"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchErrorWithMessage)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchErrorWithMessage"));
}

TEST_F(EtsTestErrorTSToETSTest, testCatchErrorWithCause)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCatchErrorWithCause"));
}

}  // namespace ark::ets::interop::js::testing