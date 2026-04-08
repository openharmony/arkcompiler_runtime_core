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

class AllEtsPromiseTsToEtsTest : public EtsInteropTest {};

TEST_F(AllEtsPromiseTsToEtsTest, checkPromiseNumber)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkPromiseNumber.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkPromiseString)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkPromiseString.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkPromiseBoolean)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkPromiseBoolean.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkPromiseBigInt)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkPromiseBigInt.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkPromiseNull)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkPromiseNull.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkPromiseUndefined)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkPromiseUndefined.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkPromiseNan)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkPromiseNan.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkPromiseInfinity)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkPromiseInfinity.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkThenTwoParamsFulfilled)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkThenTwoParamsFulfilled.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkThenTwoParamsRejected)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkThenTwoParamsRejected.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkThenOnlyOnFulfilled)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkThenOnlyOnFulfilled.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkThenNoParamsRejected)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkThenNoParamsRejected.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkCatchStandardRejected)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkCatchStandardRejected.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkCatchFulfilledPassthrough)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkCatchFulfilledPassthrough.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkCatchReturnPromise)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkCatchReturnPromise.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkCatchThrowChain)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkCatchThrowChain.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkFinallyFulfilled)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkFinallyFulfilled.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkFinallyRejectedFromTS)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkFinallyRejectedFromTS.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkFinallyNoParam)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkFinallyNoParam.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkFinallyThrowOverridesFulfilled)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkFinallyThrowOverridesFulfilled.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkFinallyThrowOverridesRejected)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkFinallyThrowOverridesRejected.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkFinallyReturnValueIgnored)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkFinallyReturnValueIgnored.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkAllFulfilled)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkAllFulfilled.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkAllRejected)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkAllRejected.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticAllEmpty)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticAllEmpty.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticAllWithPromises)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticAllWithPromises.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticAllWithRejected)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticAllWithRejected.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticAllMixed)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticAllMixed.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticAllWithString)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticAllWithString.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticAnyEmpty)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticAnyEmpty.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkAnyFulfilled)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkAnyFulfilled.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticAnyAllRejected)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticAnyAllRejected.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticAnyMixed)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticAnyMixed.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticAnyFirstWins)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticAnyFirstWins.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkRaceFulfilled)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkRaceFulfilled.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkRaceRejected)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkRaceRejected.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticRaceEmpty)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticRaceEmpty.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticRacePlainValues)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticRacePlainValues.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticRaceFulfilledWins)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticRaceFulfilledWins.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticRaceRejectedWins)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticRaceRejectedWins.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticRaceMixedImmediateWins)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticRaceMixedImmediateWins.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticRaceRejectImmediate)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticRaceRejectImmediate.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticRaceWithString)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticRaceWithString.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkThenPromiseUndefined)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkThenPromiseUndefined.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkResolvePromiseResolve)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkResolvePromiseResolve.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkResolvePromiseReject)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkResolvePromiseReject.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkcbLambdaNoWait)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkcbLambdaNoWait.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkcbLambda)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkcbLambda.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkInstanceMethodNoWait)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkInstanceMethodNoWait.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkInstanceMethod)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkInstanceMethod.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticMethodNoWait)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticMethodNoWait.ts"));
}

TEST_F(AllEtsPromiseTsToEtsTest, checkStaticMethod)
{
    ASSERT_TRUE(RunJsTestSuite("runner_checkStaticMethod.ts"));
}

}  // namespace ark::ets::interop::js::testing
